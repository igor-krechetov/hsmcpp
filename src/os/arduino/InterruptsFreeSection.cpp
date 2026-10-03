// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/common/InterruptsFreeSection.hpp"

#include <Arduino.h>

namespace hsmcpp {

InterruptsFreeSection::InterruptsFreeSection() {
    noInterrupts();  // disable interrupts
}

InterruptsFreeSection::~InterruptsFreeSection() {
    interrupts();  // enable interrupts
}

}  // namespace hsmcpp
