// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_COMMON_LOCKGUARD_HPP__
#define __HSMCPP_OS_COMMON_LOCKGUARD_HPP__

namespace hsmcpp
{

class Mutex;

class LockGuard
{
public:
    explicit LockGuard(Mutex& sync);
    ~LockGuard();

private:
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    
private:
    Mutex& mSync;
};

} // namespace hsmcpp

#endif // __HSMCPP_OS_COMMON_LOCKGUARD_HPP__
