// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/freertos/FreeRtosPort.hpp"

#ifdef BUILD_FREERTOS_DEFAULT_ISR_DETECT
BaseType_t xPortIsInsideInterrupt(void) {
    return pdFALSE;
}
#endif
