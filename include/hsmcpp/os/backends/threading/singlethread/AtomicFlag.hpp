// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_ATOMICFLAG_HPP
#define HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_ATOMICFLAG_HPP

#include "hsmcpp/os/UniqueLock.hpp"

namespace hsmcpp {

/**
 * @brief AtomicFlag implementation for single-threaded builds (HSM_DISABLE_THREADSAFETY).
 * @details With no concurrent callers there is nothing to make atomic, so the flag is a plain
 *          bool. The read-old-then-set semantics of test_and_set() are preserved because the HSM
 *          uses it as a re-entrancy guard (first call returns false and proceeds, a re-entrant
 *          call returns true and bails) -- that intent is still meaningful single-threaded.
 *          wait() never blocks (no other thread could ever wake it) and notify() is a no-op.
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

    inline bool test_and_set() noexcept {
        const bool old = mValue;
        mValue = true;
        return old;
    }

    inline void clear() noexcept {
        mValue = false;
    }

    inline bool test() const noexcept {
        return mValue;
    }

    inline UniqueLock lock() noexcept {
        return UniqueLock();
    }

    inline void wait(const bool old) noexcept {
        (void)old;
    }

    inline void notify() noexcept {}

private:
    bool mValue = false;
};

}  // namespace hsmcpp

#endif  // HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_ATOMICFLAG_HPP
