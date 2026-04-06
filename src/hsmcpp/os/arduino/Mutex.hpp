// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_STL_MUTEX_HPP
#define HSMCPP_OS_STL_MUTEX_HPP

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

#endif // HSMCPP_OS_STL_MUTEX_HPP
