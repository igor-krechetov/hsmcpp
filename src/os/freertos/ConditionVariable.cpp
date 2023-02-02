// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/freertos/ConditionVariable.hpp"

#include "hsmcpp/os/freertos/FreeRtosPort.hpp"

namespace hsmcpp {

ConditionVariable::ConditionVariable() {
    mHandle = xSemaphoreCreateBinary();
}

ConditionVariable::~ConditionVariable() {
    if (nullptr != mHandle) {
        // TODO: apply xSemaphoreGetMutexHolder if INCLUDE_xSemaphoreGetMutexHolder / configUSE_MUTEXES
        notify();

        // NOTE: Do not delete a semaphore that has tasks blocked on it
        //       (tasks that are in the Blocked state waiting for the
        //        semaphore to become available).
        vSemaphoreDelete(mHandle);
        mHandle = nullptr;
    }
}

void ConditionVariable::wait(UniqueLock& sync, std::function<bool()> stopWaiting) {
    sync.unlock();

    if (nullptr != mHandle) {
        bool locked = false;
        BaseType_t isInsideISR = xPortIsInsideInterrupt();

        while ((nullptr == stopWaiting || false == stopWaiting()) && (false == locked)) {
            if (pdTRUE == isInsideISR) {
                locked = (pdTRUE == xSemaphoreTakeFromISR(mHandle, nullptr));
            } else {
                locked = (pdTRUE == xSemaphoreTake(mHandle, portMAX_DELAY));
            }
        }
    }

    sync.lock();
}

bool ConditionVariable::wait_for(UniqueLock& sync, const int timeoutMs, std::function<bool()> stopWaiting) {
    bool locked = false;
    BaseType_t isInsideISR = xPortIsInsideInterrupt();

    sync.unlock();

    if (nullptr != mHandle) {
        if (nullptr == stopWaiting || false == stopWaiting()) {
            if (pdTRUE == isInsideISR) {
                // NOTE: timeouts are not supported inside of ISR calls
                locked = (pdTRUE == xSemaphoreTakeFromISR(mHandle, nullptr));
            } else {
                locked = (pdTRUE == xSemaphoreTake(mHandle, timeoutMs / portTICK_PERIOD_MS));
            }
        }
    }

    sync.lock();

    return locked;
}

void ConditionVariable::notify() {
    if (nullptr != mHandle) {
        if (pdTRUE == xPortIsInsideInterrupt()) {
            xSemaphoreGiveFromISR(mHandle, nullptr);
        } else {
            xSemaphoreGive(mHandle);
        }
    }
}

}  // namespace hsmcpp
