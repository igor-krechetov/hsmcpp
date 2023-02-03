// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/freertos/InterruptsFreeSection.hpp"

#include <task.h>

#include "hsmcpp/os/freertos/FreeRtosPort.hpp"

namespace hsmcpp {

// Source: https://www.freertos.org/taskENTER_CRITICAL_taskEXIT_CRITICAL.html
// The taskENTER_CRITICAL() and taskEXIT_CRITICAL() macros provide a basic
// critical section implementation that works by simply disabling interrupts,
// either globally, or up to a specific interrupt priority level.

// Source: https://www.freertos.org/taskENTER_CRITICAL_FROM_ISR_taskEXIT_CRITICAL_FROM_ISR.html
// If the FreeRTOS port being used supports interrupt nesting then calling
// taskENTER_CRITICAL_FROM_ISR() will disable interrupts at and below the
// interrupt priority set by the configMAX_SYSCALL_INTERRUPT_PRIORITY (or
// configMAX_API_CALL_INTERRUPT_PRIORITY) kernel configuration constant, and
// leave all other interrupt priorities enabled. If the FreeRTOS port being
// used does not support interrupt nesting then taskENTER_CRITICAL_FROM_ISR()
// and taskEXIT_CRITICAL_FROM_ISR() will have no effect.

InterruptsFreeSection::InterruptsFreeSection() {
    BaseType_t isInsideISR = xPortIsInsideInterrupt();

    if (pdFALSE == isInsideISR) {
        taskENTER_CRITICAL();
    } else {
        mSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    }
}

InterruptsFreeSection::~InterruptsFreeSection() {
    BaseType_t isInsideISR = xPortIsInsideInterrupt();

    if (pdFALSE == isInsideISR) {
        taskEXIT_CRITICAL();
    } else {
        taskEXIT_CRITICAL_FROM_ISR(mSavedInterruptStatus);
    }
}

}  // namespace hsmcpp
