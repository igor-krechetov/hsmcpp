// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_POSIX_CRITICALSECTION_HPP
#define HSMCPP_OS_POSIX_CRITICALSECTION_HPP

// NOTE: the purpose of this class is adding safe signals support, so this header is needed
// cppcheck-suppress misra-c2012-21.5
#include <signal.h>

namespace hsmcpp
{

class CriticalSection
{
public:
    CriticalSection();
    ~CriticalSection();

private:
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
    CriticalSection(CriticalSection&&) = delete;
    CriticalSection& operator=(CriticalSection&&) = delete;

private:
    sigset_t mOriginalSigMask;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_POSIX_CRITICALSECTION_HPP
