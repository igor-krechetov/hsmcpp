// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/stl/ConditionVariable.hpp"

namespace hsmcpp {

ConditionVariable::~ConditionVariable() {
    mVariable.notify_all();
}

void ConditionVariable::wait(UniqueLock& sync, std::function<bool()> stopWaiting) {
    std::unique_lock<std::mutex> lck;

    if (false == sync.owns_lock()) {
        lck = std::unique_lock<std::mutex>(sync.mutex()->nativeHandle(), std::try_to_lock);
    } else {
        lck = std::unique_lock<std::mutex>(sync.mutex()->nativeHandle(), std::adopt_lock);
    }

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::function has bool() operator
    if (stopWaiting) {
        mVariable.wait(lck, stopWaiting);
    } else {
        mVariable.wait(lck);
    }

    // no need to unlock on exit if lock already belonged to UniqueLock object
    if (true == sync.owns_lock()) {
        lck.release();
        sync.unlock();
    }
}

bool ConditionVariable::wait_for(UniqueLock& sync, const int timeoutMs, std::function<bool()> stopWaiting) {
    std::unique_lock<std::mutex> lck;

    if (false == sync.owns_lock()) {
        lck = std::unique_lock<std::mutex>(sync.mutex()->nativeHandle(), std::try_to_lock);
    } else {
        lck = std::unique_lock<std::mutex>(sync.mutex()->nativeHandle(), std::adopt_lock);
    }

    bool res;

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::function has bool() operator
    if (stopWaiting) {
        res = mVariable.wait_for(lck, std::chrono::milliseconds(timeoutMs), stopWaiting);
    } else {
        res = (std::cv_status::timeout != mVariable.wait_for(lck, std::chrono::milliseconds(timeoutMs)));
    }

    // no need to unlock on exit if lock already belonged to UniqueLock object
    if (true == sync.owns_lock()) {
        lck.release();
        sync.unlock();
    }

    return res;
}

}  // namespace hsmcpp
