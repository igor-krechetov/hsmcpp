// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_ARDUINO_ATOMICFLAG_HPP
#define HSMCPP_OS_ARDUINO_ATOMICFLAG_HPP

#include "hsmcpp/os/UniqueLock.hpp"

namespace hsmcpp {

/**
 * @brief AtomicFlag implementation for Arduino
 * @details Arduino is single threaded so this is just a simple wrapper for a bool flag
 *
 * @requirement HSMCPP.SWR_HSM_109
 */
class AtomicFlag {
public:
    AtomicFlag() = default;
    ~AtomicFlag() = default;
    AtomicFlag(const AtomicFlag&) = delete;
    AtomicFlag& operator=(const AtomicFlag&) = delete;
    AtomicFlag& operator=(const AtomicFlag&) volatile = delete;

    bool test_and_set() noexcept;
    void clear() noexcept;
    bool test() const noexcept;
    UniqueLock lock() noexcept;
    void wait(const bool old) const noexcept;
    void notify() noexcept;

private:
    volatile bool mValue = false;
};

}  // namespace hsmcpp

#endif  // HSMCPP_OS_ARDUINO_ATOMICFLAG_HPP
