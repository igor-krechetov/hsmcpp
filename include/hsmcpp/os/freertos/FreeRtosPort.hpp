// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_FREERTOS_FREERTOSPORT_HPP__
#define __HSMCPP_OS_FREERTOS_FREERTOSPORT_HPP__

#ifdef BUILD_FREERTOS_DEFAULT_ISR_DETECT
#include <FreeRTOS.h>

BaseType_t xPortIsInsideInterrupt(void);
#endif

#endif // __HSMCPP_OS_FREERTOS_FREERTOSPORT_HPP__