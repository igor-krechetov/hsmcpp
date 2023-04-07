// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_BASEASYNCHSM_HPP
#define HSMCPP_TESTS_HSM_BASEASYNCHSM_HPP

#include <atomic>

#include "TestsCommon.hpp"
#include "hsmcpp/os/ConditionVariable.hpp"
#include "hsmcpp/os/Mutex.hpp"

class BaseAsyncHsm {
public:
    BaseAsyncHsm();
    virtual ~BaseAsyncHsm() = default;

    void blockExecution(const std::string& msg);
    bool waitAsyncOperation();
    bool waitAsyncOperation(const bool unblockNext);
    bool waitAsyncOperation(const int timeoutMs, const bool unblockNext);
    void unblockNextStep();

protected:
    hsmcpp::Mutex mSyncLock;
    hsmcpp::ConditionVariable mSyncVariable;
    std::atomic<bool> mSyncVariableCheck;
    hsmcpp::Mutex mSyncBlockNextStep;
    hsmcpp::ConditionVariable mBlockNextStep;
};

#endif  // HSMCPP_TESTS_HSM_BASEASYNCHSM_HPP