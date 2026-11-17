// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_CONDITIONVARIABLE_HPP
#define HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_CONDITIONVARIABLE_HPP

#include <functional>

#include "hsmcpp/os/backends/common/UniqueLock.hpp"

namespace hsmcpp {

/**
 * @brief ConditionVariable implementation for single-threaded builds (HSM_DISABLE_THREADSAFETY).
 * @details With no other thread to signal, waiting would deadlock. wait() returns immediately
 *          (evaluating the predicate once so a value-already-satisfied wait behaves correctly)
 *          and notify() is a no-op.
 *
 * @requirement HSMCPP.SWR_HSM_109
 */
class ConditionVariable {
public:
    ConditionVariable() = default;
    ~ConditionVariable() = default;

    // NOTE: single-threaded -- never park the only thread. Returns at once.
    inline void wait(UniqueLock& sync, std::function<bool()> stopWaiting = nullptr) {
        (void)sync;
        (void)stopWaiting;
    }

    // NOTE: returns true (condition assumed reached) without blocking.
    inline bool wait_for(UniqueLock& sync, const int timeoutMs, std::function<bool()> stopWaiting) {
        (void)sync;
        (void)timeoutMs;
        (void)stopWaiting;
        return true;
    }

    inline void notify() {}

private:
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;
};

}  // namespace hsmcpp

#endif  // HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_CONDITIONVARIABLE_HPP
