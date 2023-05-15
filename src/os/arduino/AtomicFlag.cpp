// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/os/arduino/AtomicFlag.hpp"

namespace hsmcpp {

bool AtomicFlag::test_and_set() noexcept {
    const bool old = mValue;

    mValue = true;
    return old;
}

void AtomicFlag::clear() noexcept {
    mValue = false;
}

bool AtomicFlag::test() const noexcept {
    return mValue;
}

UniqueLock AtomicFlag::lock() noexcept {
    return UniqueLock();
}

void AtomicFlag::wait(const bool old) const noexcept {
    // do nothing
}

void AtomicFlag::notify() noexcept {
    // do nothing
}

}  // namespace hsmcpp
