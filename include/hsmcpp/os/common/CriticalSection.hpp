// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_COMMON_CRITICALSECTION_HPP
#define HSMCPP_OS_COMMON_CRITICALSECTION_HPP

namespace hsmcpp
{

class Mutex;
class InterruptsFreeSection;

class CriticalSection
{
public:
    explicit CriticalSection(Mutex& sync);
    ~CriticalSection();

private:
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
    CriticalSection(CriticalSection&&) = delete;
    CriticalSection& operator=(CriticalSection&&) = delete;

private:
    Mutex& mSync;
    InterruptsFreeSection* mInterruptsBlock = nullptr;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_COMMON_CRITICALSECTION_HPP
