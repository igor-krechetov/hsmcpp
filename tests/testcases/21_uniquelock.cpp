// Copyright (C) 2021 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <thread>

#include "TestsCommon.hpp"
#include "hsmcpp/os/Mutex.hpp"
#include "hsmcpp/os/UniqueLock.hpp"


TEST(uniquelock, uniquelock_move_construct) {
    TEST_EXCLUDE("platform abstraction primitive");
    TEST_DESCRIPTION("move-construct transfers ownership and empties the source (both ownership branches)");

    // Branch 1: source currently owns the lock -> ownership transfers to destination.
    {
        Mutex m;
        UniqueLock a(m);

        ASSERT_TRUE(a.owns_lock());
        ASSERT_EQ(a.mutex(), &m);

        UniqueLock b(std::move(a));

        // destination took over ownership of the mutex
        EXPECT_TRUE(b.owns_lock());
        EXPECT_EQ(b.mutex(), &m);

        // source was emptied
        EXPECT_FALSE(a.owns_lock());
        EXPECT_EQ(a.mutex(), nullptr);
    }

    // Branch 2: source is default-constructed (unowned) -> the "else" branch of
    // the move ctor runs; destination ends up empty and unowned too.
    {
        UniqueLock src;  // default-constructed: no mutex, not owning

        ASSERT_FALSE(src.owns_lock());
        ASSERT_EQ(src.mutex(), nullptr);

        UniqueLock dst(std::move(src));

        EXPECT_FALSE(dst.owns_lock());
        EXPECT_EQ(dst.mutex(), nullptr);

        EXPECT_FALSE(src.owns_lock());
        EXPECT_EQ(src.mutex(), nullptr);
    }
}

TEST(uniquelock, uniquelock_move_assign_dest_owns) {
    TEST_EXCLUDE("platform abstraction primitive");
    TEST_DESCRIPTION("move-assign releases a destination that already owns, transfers ownership, and empties the source");

    // Branch 1: destination already owns a (different) mutex -> the "destination
    // already owns" branch runs: the former destination mutex is unlocked before
    // the source's mutex/ownership is taken over.
    {
        Mutex m1;  // source's mutex
        Mutex m2;  // destination's original mutex

        UniqueLock src(m1);
        UniqueLock dst(m2);

        ASSERT_TRUE(src.owns_lock());
        ASSERT_EQ(src.mutex(), &m1);
        ASSERT_TRUE(dst.owns_lock());
        ASSERT_EQ(dst.mutex(), &m2);

        dst = std::move(src);

        // destination took over the source's mutex and ownership
        EXPECT_TRUE(dst.owns_lock());
        EXPECT_EQ(dst.mutex(), &m1);

        // source was emptied
        EXPECT_FALSE(src.owns_lock());
        EXPECT_EQ(src.mutex(), nullptr);

        // the former destination mutex (m2) was released by the assignment - so it
        // can be re-locked without deadlocking. try_lock must succeed; unlock it.
        EXPECT_TRUE(m2.nativeHandle().try_lock());
        m2.nativeHandle().unlock();
    }

    // Branch 2: destination is empty (default-constructed) -> the dest-owns branch
    // is skipped (mSync == nullptr); ownership simply transfers from the source.
    {
        Mutex m;

        UniqueLock src(m);
        UniqueLock dst;  // default-constructed: no mutex, not owning

        ASSERT_TRUE(src.owns_lock());
        ASSERT_EQ(src.mutex(), &m);
        ASSERT_FALSE(dst.owns_lock());
        ASSERT_EQ(dst.mutex(), nullptr);

        dst = std::move(src);

        // destination took over ownership of the mutex
        EXPECT_TRUE(dst.owns_lock());
        EXPECT_EQ(dst.mutex(), &m);

        // source was emptied
        EXPECT_FALSE(src.owns_lock());
        EXPECT_EQ(src.mutex(), nullptr);
    }
}

TEST(uniquelock, uniquelock_release) {
    TEST_EXCLUDE("platform abstraction primitive");
    TEST_DESCRIPTION("release() relinquishes ownership and empties the lock without unlocking the underlying mutex");

    Mutex m;
    UniqueLock lock(m);

    ASSERT_TRUE(lock.owns_lock());
    ASSERT_EQ(lock.mutex(), &m);

    Mutex* released = lock.release();

    // release() returns the owned mutex and empties the UniqueLock...
    EXPECT_EQ(released, &m);
    EXPECT_FALSE(lock.owns_lock());
    EXPECT_EQ(lock.mutex(), nullptr);

    // ...but it does NOT unlock the mutex. A worker thread must fail to acquire it
    // because the mutex is still held after release().
    bool workerAcquired = true;
    std::thread worker([&]() { workerAcquired = m.nativeHandle().try_lock(); });
    worker.join();
    EXPECT_FALSE(workerAcquired);

    // Unlock manually exactly once (confirming release() did not unlock it). After
    // this the mutex is free again and can be re-locked without deadlocking.
    m.unlock();
    EXPECT_TRUE(m.nativeHandle().try_lock());
    m.nativeHandle().unlock();
}
