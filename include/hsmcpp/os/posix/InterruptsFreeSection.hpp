// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_POSIX_INTERRUPTSFREESECTION_HPP
#define HSMCPP_OS_POSIX_INTERRUPTSFREESECTION_HPP

// NOTE: the purpose of this class is adding safe signals support, so this header is needed
// cppcheck-suppress misra-c2012-21.5
#include <signal.h>

namespace hsmcpp
{

class InterruptsFreeSection
{
public:
    InterruptsFreeSection();
    ~InterruptsFreeSection();

private:
    InterruptsFreeSection(const InterruptsFreeSection&) = delete;
    InterruptsFreeSection& operator=(const InterruptsFreeSection&) = delete;
    InterruptsFreeSection(InterruptsFreeSection&&) = delete;
    InterruptsFreeSection& operator=(InterruptsFreeSection&&) = delete;

private:
    sigset_t mOriginalSigMask;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_POSIX_INTERRUPTSFREESECTION_HPP
