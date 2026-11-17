// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_CONDITIONVARIABLE_HPP
#define HSMCPP_OS_CONDITIONVARIABLE_HPP

#include "os.hpp"

// Resolves to backends/threading/<backend>/ConditionVariable.hpp (see os.hpp).
#include HSM_THREADING_HEADER(ConditionVariable.hpp)

#endif  // HSMCPP_OS_CONDITIONVARIABLE_HPP
