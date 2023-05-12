// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/os/stl/AtomicFlag.hpp"

#include "hsmcpp/os/UniqueLock.hpp"

namespace hsmcpp {

AtomicFlag::AtomicFlag()
    : mValue(false) {}

bool AtomicFlag::test_and_set() noexcept {
    return mValue.exchange(true);
}

void AtomicFlag::clear() noexcept {
    mValue.store(false);
}

bool AtomicFlag::test() const noexcept {
    return mValue.load();
}

UniqueLock AtomicFlag::lock() noexcept {
    return UniqueLock(mSync);
}

void AtomicFlag::wait(const bool old) noexcept {
    if (mValue.load() == old) {
        UniqueLock lk(mSync);

        // cppcheck-suppress misra-c2012-15.5 ; false-positive. "return" statement belongs to lambda function, not parent function
        mWaitCond.wait(lk, [&]() { return (mValue.load() != old); });
    }
}

void AtomicFlag::notify() noexcept {
    mWaitCond.notify();
}

}  // namespace hsmcpp
