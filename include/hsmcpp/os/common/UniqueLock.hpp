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
    UniqueLock(void) = default;
    explicit UniqueLock(Mutex& sync);
    ~UniqueLock();

    UniqueLock(UniqueLock&& src) noexcept;

    UniqueLock& operator=(UniqueLock&& src) noexcept;

    void lock(void);
    void unlock(void);

    inline bool owns_lock(void) const noexcept
    {
        return mOwnsLock;
    }

    inline explicit operator bool(void) const noexcept
    {
        return owns_lock();
    }

    Mutex* release(void) noexcept;
    inline Mutex* mutex(void) const noexcept
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
