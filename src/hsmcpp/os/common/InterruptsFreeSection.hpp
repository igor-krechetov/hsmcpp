// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_COMMON_INTERRUPTSFREESECTION_HPP
#define HSMCPP_OS_COMMON_INTERRUPTSFREESECTION_HPP

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
};

} // namespace hsmcpp

#endif // HSMCPP_OS_COMMON_INTERRUPTSFREESECTION_HPP
