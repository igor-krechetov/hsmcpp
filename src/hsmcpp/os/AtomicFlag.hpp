// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_ATOMICFLAG_HPP
#define HSMCPP_OS_ATOMICFLAG_HPP

#include "os.hpp"

#if defined(FREERTOS_AVAILABLE)
 #include "freertos/AtomicFlag.hpp"
#elif defined(PLATFORM_ARDUINO)
  #include "arduino/AtomicFlag.hpp"
#elif defined(STL_AVAILABLE)
  #include "stl/AtomicFlag.hpp"
#else
 #error PLATFORM not supported
#endif

#endif // HSMCPP_OS_ATOMICFLAG_HPP
