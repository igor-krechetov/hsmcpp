// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_MUTEX_HPP
#define HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_MUTEX_HPP

namespace hsmcpp {

/**
 * @brief Mutex implementation for single-threaded builds (HSM_DISABLE_THREADSAFETY).
 * @details There is no concurrent access to guard against, so lock()/unlock() are no-ops.
 *
 * @requirement HSMCPP.SWR_HSM_109
 */
class Mutex {
public:
    Mutex() = default;
    ~Mutex() = default;

    inline void lock() {}

    inline void unlock() {}

private:
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};

}  // namespace hsmcpp

#endif  // HSMCPP_OS_BACKENDS_THREADING_SINGLETHREAD_MUTEX_HPP
