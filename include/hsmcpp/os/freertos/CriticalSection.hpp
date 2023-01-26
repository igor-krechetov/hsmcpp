// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_FREERTOS_CRITICALSECTION_HPP
#define HSMCPP_OS_FREERTOS_CRITICALSECTION_HPP

#include <FreeRTOS.h>

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

private:
    UBaseType_t mSavedInterruptStatus;
};

} // namespace hsmcpp

#endif // HSMCPP_OS_FREERTOS_CRITICALSECTION_HPP
