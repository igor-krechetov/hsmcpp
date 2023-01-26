// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_CONDITIONVARIABLE_HPP
#define HSMCPP_OS_CONDITIONVARIABLE_HPP

#include "os.hpp"

#if defined(FREERTOS_AVAILABLE)
 #include "freertos/ConditionVariable.hpp"
#elif defined(STL_AVAILABLE)
 #include "stl/ConditionVariable.hpp"
#elif defined(PLATFORM_ARDUINO)
  #include "arduino/ConditionVariable.hpp"
#else
 #error PLATFORM not supported
#endif

#endif // HSMCPP_OS_CONDITIONVARIABLE_HPP
