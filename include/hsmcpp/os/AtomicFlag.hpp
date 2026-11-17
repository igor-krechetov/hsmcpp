// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_ATOMICFLAG_HPP
#define HSMCPP_OS_ATOMICFLAG_HPP

#include "os.hpp"

// Resolves to backends/threading/<backend>/AtomicFlag.hpp (see os.hpp).
#include HSM_THREADING_HEADER(AtomicFlag.hpp)

#endif  // HSMCPP_OS_ATOMICFLAG_HPP
