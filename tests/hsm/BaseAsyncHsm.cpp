// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "BaseAsyncHsm.hpp"
#include "hsmcpp/os/UniqueLock.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "BaseAsyncHsm"

BaseAsyncHsm::BaseAsyncHsm() {
    mSyncVariableCheck = false;
}

void BaseAsyncHsm::blockExecution(const std::string& msg) {
    if (true == mEnableSyncMode) {
        UniqueLock lck(mSyncLock);

        // printf("----> on%s: wait\n", msg.c_str());
        mSyncVariableCheck = true;
        mSyncVariable.notify();
        ASSERT_TRUE(mBlockNextStep.wait_for(lck, 5000, [&]() { return (false == mSyncVariableCheck.load()); }));
        mSyncVariableCheck = false;
        // printf("----> on%s: done\n", msg.c_str());
    }
}

bool BaseAsyncHsm::waitAsyncOperation() {
    return waitAsyncOperation(true);
}

bool BaseAsyncHsm::waitAsyncOperation(const bool unblockNext) {
    return waitAsyncOperation(5000, unblockNext);
}

bool BaseAsyncHsm::waitAsyncOperation(const int timeoutMs, const bool unblockNext) {
    UniqueLock lck(mSyncLock);
    bool res = true;

    // printf("----> wait BEGIN\n");
    if (false == mSyncVariableCheck.load()) {
        res = mSyncVariable.wait_for(lck, timeoutMs, [&]() { return (true == mSyncVariableCheck.load()); });
    }
    // printf("----> wait DONE (res=%s)\n", BOOL2STR(res));

    if ((true == unblockNext) || (false == res)) {
        unblockNextStep();
    }

    return res;
}

void BaseAsyncHsm::unblockNextStep() {
    HSM_TRACE_CALL_DEBUG_ARGS("---->");
    mSyncVariableCheck = false;
    // printf("-----> notify\n");
    mBlockNextStep.notify();
    // printf("-----> notify done\n");
}

void BaseAsyncHsm::setSyncMode(const bool enable) {
    mEnableSyncMode = enable;
}