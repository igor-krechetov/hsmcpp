// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_INTERRUPTSFREESECTION_HPP
#define HSMCPP_OS_INTERRUPTSFREESECTION_HPP

#include "os.hpp"

// Resolves to backends/interrupts/<backend>/InterruptsFreeSection.hpp (see os.hpp).
// The interrupts backend always follows the real platform and is never no-op'd.
#include HSM_INTERRUPTS_HEADER(InterruptsFreeSection.hpp)

#endif  // HSMCPP_OS_INTERRUPTSFREESECTION_HPP
