// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_CONDITIONVARIABLE_HPP__
#define __HSMCPP_OS_CONDITIONVARIABLE_HPP__

#include "os.hpp"

#ifdef FREERTOS_AVAILABLE
 #include "freertos/ConditionVariable.hpp"
#elif STL_AVAILABLE
 #include "stl/ConditionVariable.hpp"
#else
 #error PLATFORM not supported
#endif

#endif // __HSMCPP_OS_CONDITIONVARIABLE_HPP__
