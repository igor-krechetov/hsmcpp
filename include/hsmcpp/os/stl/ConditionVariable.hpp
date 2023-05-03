// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_OS_STL_CONDITIONVARIABLE_HPP
#define HSMCPP_OS_STL_CONDITIONVARIABLE_HPP

#include "hsmcpp/os/common/UniqueLock.hpp"
#include "Mutex.hpp"
#include <condition_variable>
#include <functional>

namespace hsmcpp
{

class ConditionVariable {
public:
    ConditionVariable() = default;
    ~ConditionVariable();

    void wait(UniqueLock& sync, const std::function<bool()>& stopWaiting = nullptr);
    bool wait_for(UniqueLock& sync, const int timeoutMs, const std::function<bool()>& stopWaiting);
    inline void notify();

private:
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

private:
    std::condition_variable mVariable;
};

inline void ConditionVariable::notify()
{
    mVariable.notify_all();
}

} // namespace hsmcpp

#endif // HSMCPP_OS_STL_CONDITIONVARIABLE_HPP
