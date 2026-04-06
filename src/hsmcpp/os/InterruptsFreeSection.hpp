// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_INTERRUPTSFREESECTION_HPP
#define HSMCPP_OS_INTERRUPTSFREESECTION_HPP

#include "os.hpp"

#if defined(FREERTOS_AVAILABLE)
 #include "freertos/InterruptsFreeSection.hpp"
#elif defined(PLATFORM_ARDUINO)
 #include "common/InterruptsFreeSection.hpp"
#elif defined(POSIX_AVAILABLE)
 #include "posix/InterruptsFreeSection.hpp"
#elif defined(PLATFORM_WINDOWS)
 #include "common/InterruptsFreeSection.hpp"
#else
 #error PLATFORM not supported
#endif

#endif // HSMCPP_OS_INTERRUPTSFREESECTION_HPP
