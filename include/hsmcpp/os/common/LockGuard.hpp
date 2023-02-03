// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_COMMON_LOCKGUARD_HPP
#define HSMCPP_OS_COMMON_LOCKGUARD_HPP

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
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;

private:
    Mutex& mSync;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_COMMON_LOCKGUARD_HPP
