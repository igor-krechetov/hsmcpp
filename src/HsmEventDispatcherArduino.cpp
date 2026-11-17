// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherArduino.hpp"

#include <Arduino.h>

#include "hsmcpp/logging.hpp"

namespace hsmcpp {

constexpr const char* HSM_TRACE_CLASS = "HsmEventDispatcherArduino";

// TODO: this dispatcher needs testing with interrupts (events, timers)

HsmEventDispatcherArduino::HsmEventDispatcherArduino(const size_t eventsCacheSize)
    : HsmEventDispatcherSingleThread(eventsCacheSize) {
    HSM_TRACE_CALL();
}

HsmEventDispatcherArduino::~HsmEventDispatcherArduino() {
    HSM_TRACE_CALL();
}

std::shared_ptr<HsmEventDispatcherArduino> HsmEventDispatcherArduino::create(const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherArduino>(new HsmEventDispatcherArduino(eventsCacheSize),
                                                      &HsmEventDispatcherBase::handleDelete);
}

unsigned long HsmEventDispatcherArduino::getTickMs() const {
    return millis();
}

}  // namespace hsmcpp
