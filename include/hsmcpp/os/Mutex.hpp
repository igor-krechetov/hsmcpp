// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_MUTEX_HPP__
#define __HSMCPP_OS_MUTEX_HPP__

#include "os.hpp"

#ifdef FREERTOS_AVAILABLE
 #include "freertos/Mutex.hpp"
#elif STL_AVAILABLE
 #include "stl/Mutex.hpp"
#else
 #error PLATFORM not supported
#endif

#endif // __HSMCPP_OS_MUTEX_HPP__
