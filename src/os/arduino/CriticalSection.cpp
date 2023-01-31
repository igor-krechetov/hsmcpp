// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/common/CriticalSection.hpp"
#include <Arduino.h>

namespace hsmcpp
{
CriticalSection::CriticalSection()
{
    noInterrupts();// disable interrupts
}

CriticalSection::~CriticalSection()
{
    interrupts();// enable interrupts
}

} // namespace hsmcpp
