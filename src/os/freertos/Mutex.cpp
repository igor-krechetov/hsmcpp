// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/freertos/Mutex.hpp"
#include "hsmcpp/os/freertos/FreeRtosPort.hpp"

namespace hsmcpp
{

Mutex::Mutex()
{
    mHandle = xSemaphoreCreateMutex();
}

Mutex::~Mutex()
{
    if (nullptr != mHandle)
    {
        // NOTE: Do not delete a semaphore that has tasks blocked on it 
        //       (tasks that are in the Blocked state waiting for the
        //        semaphore to become available).
        vSemaphoreDelete(mHandle);
        mHandle = nullptr;
    }
}

void Mutex::lock()
{
    if (nullptr != mHandle)
    {
        bool locked;
        BaseType_t isInsideISR = xPortIsInsideInterrupt();
    
        do
        {
            if (pdTRUE == isInsideISR)
            {
                locked = (pdTRUE == xSemaphoreTakeFromISR(mHandle, nullptr));
            }
            else
            {
                locked = (pdTRUE == xSemaphoreTake(mHandle, portMAX_DELAY));
            }
        } while(false == locked);
    }
}

void Mutex::unlock()
{
    if (nullptr != mHandle)
    {
        xSemaphoreGive(mHandle);
    }
}

} // namespace hsmcpp
