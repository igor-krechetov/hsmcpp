// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_FREERTOS_INTERRUPTSFREESECTION_HPP
#define HSMCPP_OS_FREERTOS_INTERRUPTSFREESECTION_HPP

#include <FreeRTOS.h>

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
    UBaseType_t mSavedInterruptStatus;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_FREERTOS_INTERRUPTSFREESECTION_HPP
