// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_COMMON_UNIQUELOCK_HPP
#define HSMCPP_OS_COMMON_UNIQUELOCK_HPP

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
        return mOwnsLock;
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
    bool mOwnsLock = false;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_COMMON_UNIQUELOCK_HPP
