// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "AsyncHsm.hpp"

#include <chrono>

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/UniqueLock.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "AsyncHsm"

AsyncHsm::AsyncHsm()
    : HierarchicalStateMachine(AsyncHsmState::A) {}

AsyncHsm::~AsyncHsm() {}

void AsyncHsm::SetUp() {
    mSyncVariableCheck = false;
}

void AsyncHsm::TearDown() {
    HSM_TRACE_CALL_DEBUG_ARGS("----> TearDown AsyncHsm");
    mSyncVariable.notify();
    mBlockNextStep.notify();
    RELEASE_HSM();
}

bool AsyncHsm::onExit() {
    HSM_TRACE_CALL_DEBUG_ARGS("----> AsyncHsm::onExit");
    UniqueLock lck(mSyncBlockNextStep);

    HSM_TRACE_DEBUG("----> notify_one");
    mSyncVariableCheck = true;
    mSyncVariable.notify();

    HSM_TRACE_DEBUG("----> AsyncHsm::onExit: wait");
    mBlockNextStep.wait(lck);
    HSM_TRACE_DEBUG("----> AsyncHsm::onExit: done");

    return true;
}

bool AsyncHsm::onEnter(const VariantVector_t& args) {
    UniqueLock lck(mSyncBlockNextStep);

    HSM_TRACE_CALL_DEBUG_ARGS("----> AsyncHsm::onEnter");
    mSyncVariableCheck = true;
    mSyncVariable.notify();

    HSM_TRACE_DEBUG("----> AsyncHsm::onEnter: wait");
    mBlockNextStep.wait(lck);
    HSM_TRACE_DEBUG("----> AsyncHsm::onEnter: done");

    return true;
}

void AsyncHsm::onStateChanged(const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("----> AsyncHsm::onStateChanged");
    UniqueLock lck(mSyncBlockNextStep);

    HSM_TRACE_DEBUG("----> notify_one");
    mSyncVariableCheck = true;
    mSyncVariable.notify();

    HSM_TRACE_DEBUG("----> AsyncHsm::onStateChanged: wait");
    mBlockNextStep.wait(lck);
    HSM_TRACE_DEBUG("----> AsyncHsm::onStateChanged: done");
}

void AsyncHsm::onNextStateTransition(const VariantVector_t& args) {
    UniqueLock lck(mSyncBlockNextStep);

    HSM_TRACE_CALL_DEBUG_ARGS("----> AsyncHsm::onNextStateTransition");
    mSyncVariableCheck = true;
    mSyncVariable.notify();

    mBlockNextStep.wait(lck);
}

bool AsyncHsm::waitAsyncOperation(const int timeoutMs, const bool unblockNext) {
    UniqueLock lck(mSyncLock);

    HSM_TRACE_CALL_DEBUG_ARGS("----> AsyncHsm::waitAsyncOperation");
    bool res = mSyncVariable.wait_for(lck, timeoutMs, [&]() { return mSyncVariableCheck.load(); });
    mSyncVariableCheck = false;
    HSM_TRACE_DEBUG("----> AsyncHsm::waitAsyncOperation: DONE (res=%s)", BOOL2STR(res));

    if (true == unblockNext) {
        unblockNextStep();
    }

    return res;
}

void AsyncHsm::unblockNextStep() {
    HSM_TRACE_CALL_DEBUG_ARGS("----> AsyncHsm::unblockNextStep");
    mBlockNextStep.notify();
}