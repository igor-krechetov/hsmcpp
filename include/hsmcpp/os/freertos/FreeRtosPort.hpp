// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_FREERTOS_FREERTOSPORT_HPP
#define HSMCPP_OS_FREERTOS_FREERTOSPORT_HPP

#ifdef BUILD_FREERTOS_DEFAULT_ISR_DETECT
#include <FreeRTOS.h>

BaseType_t xPortIsInsideInterrupt(void);
#endif

#endif // HSMCPP_OS_FREERTOS_FREERTOSPORT_HPP