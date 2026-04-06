// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/common/UniqueLock.hpp"

#include "hsmcpp/os/Mutex.hpp"

namespace hsmcpp {

UniqueLock::UniqueLock(Mutex& sync)
    // NOTE: false-positive. thinks that ':' is arithmetic operation
    // cppcheck-suppress misra-c2012-10.4
    : mSync(&sync) {
    lock();
}

UniqueLock::~UniqueLock() {
    unlock();
}

UniqueLock::UniqueLock(UniqueLock&& src) noexcept
    : mSync(src.mSync) {
    if (true == src.mOwnsLock.test_and_set(std::memory_order_acquire)) {
        mOwnsLock.test_and_set(std::memory_order_release);
    } else {
        mOwnsLock.clear(std::memory_order_release);
    }

    src.mSync = nullptr;
    src.mOwnsLock.clear(std::memory_order_release);
}

UniqueLock& UniqueLock::operator=(UniqueLock&& src) noexcept {
    if (this != &src) {
        if (nullptr != mSync) {
            if (true == mOwnsLock.test_and_set(std::memory_order_acquire)) {
                mSync->unlock();
            } else {
                mOwnsLock.clear(std::memory_order_release);
            }
        }

        mSync = src.mSync;

        if (true == src.mOwnsLock.test_and_set(std::memory_order_acquire)) {
            mOwnsLock.test_and_set(std::memory_order_release);
        } else {
            mOwnsLock.clear(std::memory_order_release);
        }

        src.mSync = nullptr;
        src.mOwnsLock.clear(std::memory_order_release);
    }

    return *this;
}

void UniqueLock::lock() {
    if ((nullptr != mSync) && (false == mOwnsLock.test_and_set(std::memory_order_acq_rel))) {
        mSync->lock();
    }
}

void UniqueLock::unlock() {
    if (nullptr != mSync) {
        if (true == mOwnsLock.test_and_set(std::memory_order_acquire)) {
            mSync->unlock();
            mOwnsLock.clear(std::memory_order_release);
        } else {
            mOwnsLock.clear(std::memory_order_release);
        }
    }
}

Mutex* UniqueLock::release() noexcept {
    Mutex* res = mSync;

    mSync = nullptr;
    mOwnsLock.clear(std::memory_order_release);

    return res;
}

}  // namespace hsmcpp
