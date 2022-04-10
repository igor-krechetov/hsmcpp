// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_FREERTOS_CRITICALSECTION_HPP__
#define __HSMCPP_OS_FREERTOS_CRITICALSECTION_HPP__

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

#endif // __HSMCPP_OS_FREERTOS_CRITICALSECTION_HPP__
