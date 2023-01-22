// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifdef PLATFORM_ARDUINO

#include "hsmcpp/logging.hpp"
#include <stdarg.h>
#include <Arduino.h>
#include <ArduinoLog.h>

void serialPrintf(const char *fmt, ...)
{
    char buffer[1024] = {0};
    va_list args;

    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    // Serial.println(buffer);
    Log.traceln(buffer);
}

#endif
