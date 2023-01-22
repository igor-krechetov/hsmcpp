// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_STL_MUTEX_HPP__
#define __HSMCPP_OS_STL_MUTEX_HPP__

namespace hsmcpp
{

class Mutex
{
public:
    Mutex() = default;
    ~Mutex() = default;

    inline void lock()
    {
    }

    inline void unlock()
    {
    }

private:
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};

} // namespace hsmcpp

#endif // __HSMCPP_OS_STL_MUTEX_HPP__
