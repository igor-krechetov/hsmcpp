// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/os/freertos/AtomicFlag.hpp"

#include <FreeRTOS.h>
#include <atomic.h>

#include "hsmcpp/os/UniqueLock.hpp"

namespace hsmcpp {

bool AtomicFlag::test_and_set() noexcept {
    // T T F -> T (ATOMIC_COMPARE_AND_SWAP_FAILURE)
    // F T F -> F (ATOMIC_COMPARE_AND_SWAP_SUCCESS)
    return (ATOMIC_COMPARE_AND_SWAP_FAILURE == Atomic_CompareAndSwap_u32(&mValue, 1U, 0) ? true : false);
}

void AtomicFlag::clear() noexcept {
    mValue = 0;
}

bool AtomicFlag::test() const noexcept {
    return (0 != mValue);
}

UniqueLock AtomicFlag::lock() noexcept {
    return UniqueLock(mSync);
}

void AtomicFlag::wait(const bool old) noexcept {
    if (old == mValue) {
        UniqueLock lk(mSync);

        mWaitCond.wait(lk, [&]() { return (mValue != old); });
    }
}

void AtomicFlag::notify() noexcept {
    mWaitCond.notify();
}

}  // namespace hsmcpp
