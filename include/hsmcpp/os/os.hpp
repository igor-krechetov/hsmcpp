// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_OS_HPP
#define HSMCPP_OS_OS_HPP

// TODO: check if there are better ways to detect FreeRTOS
#if defined(PLATFORM_FREERTOS) || defined(INC_FREERTOS_H)
 #define FREERTOS_AVAILABLE
#elif defined(PLATFORM_POSIX) || defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (__linux__)
 #define POSIX_AVAILABLE    1
 #define STL_AVAILABLE      1
#elif defined(PLATFORM_WINDOWS) || defined(WIN32)
 #define STL_AVAILABLE      1
#endif

#endif // HSMCPP_OS_OS_HPP