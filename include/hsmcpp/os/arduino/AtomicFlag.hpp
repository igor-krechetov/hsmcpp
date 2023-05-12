// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_ARDUINO_ATOMICFLAG_HPP
#define HSMCPP_OS_ARDUINO_ATOMICFLAG_HPP

#include "hsmcpp/os/UniqueLock.hpp"

namespace hsmcpp {

/// Arduino is single threaded so this is just a simple wrapper for a bool flag
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
