// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_COMMON_UNIQUELOCK_HPP
#define HSMCPP_OS_COMMON_UNIQUELOCK_HPP

#include <atomic>

namespace hsmcpp
{

class Mutex;

class UniqueLock
{
public:
    UniqueLock() = default;
    explicit UniqueLock(Mutex& sync);
    ~UniqueLock();

    UniqueLock(UniqueLock&& src) noexcept;

    UniqueLock& operator=(UniqueLock&& src) noexcept;

    void lock();
    void unlock();

    inline bool owns_lock() const noexcept
    {
        const bool isLocked = const_cast<std::atomic_flag&>(mOwnsLock).test_and_set(std::memory_order_acquire);

        if (false == isLocked) {
            const_cast<std::atomic_flag&>(mOwnsLock).clear(std::memory_order_release);
        }
        
        return isLocked;
    }

    inline explicit operator bool() const noexcept
    {
        return owns_lock();
    }

    Mutex* release() noexcept;
    inline Mutex* mutex() const noexcept
    {
        return mSync;
    }

private:
    UniqueLock(const UniqueLock& src) = delete;
    UniqueLock& operator=(const UniqueLock& src) = delete;

private:
    Mutex* mSync = nullptr;
    mutable std::atomic_flag mOwnsLock = ATOMIC_FLAG_INIT;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_COMMON_UNIQUELOCK_HPP
