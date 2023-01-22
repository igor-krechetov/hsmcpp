// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __HSMCPP_OS_STL_CONDITIONVARIABLE_HPP__
#define __HSMCPP_OS_STL_CONDITIONVARIABLE_HPP__

#include "hsmcpp/os/common/UniqueLock.hpp"
#include <functional>

namespace hsmcpp
{

class ConditionVariable {
public:
    ConditionVariable() = default;
    ~ConditionVariable();

    void wait(UniqueLock& sync, std::function<bool()> stopWaiting = nullptr);
    bool wait_for(UniqueLock& sync, const int timeoutMs, std::function<bool()> stopWaiting);
    inline void notify();

private:
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

private:
};

inline void ConditionVariable::notify()
{
    // mVariable.notify_all();
}

} // namespace hsmcpp

#endif // __HSMCPP_OS_STL_CONDITIONVARIABLE_HPP__
