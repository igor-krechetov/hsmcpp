// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_STL_MUTEX_HPP__
#define __HSMCPP_OS_STL_MUTEX_HPP__

#include <mutex>

namespace hsmcpp
{

class Mutex
{
public:
    Mutex() = default;
    ~Mutex() = default;

    inline void lock()
    {
        mSync.lock();
    }

    inline void unlock()
    {
        mSync.unlock();
    }

    inline std::mutex& nativeHandle()
    {
        return mSync;
    }

private:
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

private:
    std::mutex mSync;
};

} // namespace hsmcpp

#endif // __HSMCPP_OS_STL_MUTEX_HPP__
