// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_MUTEX_HPP
#define HSMCPP_OS_MUTEX_HPP

#include "os.hpp"

// Resolves to backends/threading/<backend>/Mutex.hpp (see os.hpp).
#include HSM_THREADING_HEADER(Mutex.hpp)

#endif  // HSMCPP_OS_MUTEX_HPP
