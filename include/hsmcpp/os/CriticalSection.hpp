// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_CRITICALSECTION_HPP
#define HSMCPP_OS_CRITICALSECTION_HPP

#include "os.hpp"

#if defined(FREERTOS_AVAILABLE)
 #include "freertos/CriticalSection.hpp"
#elif defined(PLATFORM_ARDUINO)
 #include "common/CriticalSection.hpp"
#elif defined(POSIX_AVAILABLE)
 #include "posix/CriticalSection.hpp"
#elif defined(PLATFORM_WINDOWS)
 #include "common/CriticalSection.hpp"
#else
 #error PLATFORM not supported
#endif

#endif // HSMCPP_OS_CRITICALSECTION_HPP
