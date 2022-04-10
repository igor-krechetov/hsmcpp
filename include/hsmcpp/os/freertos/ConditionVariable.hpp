// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_FREERTOS_CONDITIONVARIABLE_HPP__
#define __HSMCPP_OS_FREERTOS_CONDITIONVARIABLE_HPP__

#include <FreeRTOS.h>
#include <semphr.h>
#include <functional>
#include "hsmcpp/os/common/UniqueLock.hpp"

#if (configUSE_MUTEXES != 1)
 #error configUSE_MUTEXES feature is required
#endif
#if (configSUPPORT_DYNAMIC_ALLOCATION != 1)
 #error configSUPPORT_DYNAMIC_ALLOCATION feature is required
#endif 

namespace hsmcpp
{

class ConditionVariable {
public:
    ConditionVariable();
    ~ConditionVariable();

    void wait(UniqueLock& sync, std::function<bool()> stopWaiting = nullptr);
    bool wait_for(UniqueLock& sync, const int timeoutMs, std::function<bool()> stopWaiting);
    void notify();

private:
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

private:
    SemaphoreHandle_t mHandle = nullptr;
};

} // namespace hsmcpp

#endif // __HSMCPP_OS_FREERTOS_CONDITIONVARIABLE_HPP__
