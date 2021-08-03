#include "AsyncHsm.hpp"
#include "hsmcpp/logging.hpp"
#include <chrono>

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "AsyncHsm"

AsyncHsm::AsyncHsm() : HierarchicalStateMachine(AsyncHsmState::A)
{
}

AsyncHsm::~AsyncHsm()
{}

void AsyncHsm::SetUp()
{
    mSyncVariableCheck = false;
}

void AsyncHsm::TearDown()
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("----> TearDown AsyncHsm");
    mSyncVariable.notify_all();
    mBlockNextStep.notify_all();
    RELEASE_HSM();
}

bool AsyncHsm::onExit()
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onExit");
    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);

    __HSM_TRACE_DEBUG__("----> notify_one");
    mSyncVariableCheck = true;
    mSyncVariable.notify_one();

    __HSM_TRACE_DEBUG__("----> AsyncHsm::onExit: wait");
    mBlockNextStep.wait(lck);
    __HSM_TRACE_DEBUG__("----> AsyncHsm::onExit: done");

    return true;
}

bool AsyncHsm::onEnter(const VariantList_t& args)
{
    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);

    __HSM_TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onEnter");
    mSyncVariableCheck = true;
    mSyncVariable.notify_one();

    __HSM_TRACE_DEBUG__("----> AsyncHsm::onEnter: wait");
    mBlockNextStep.wait(lck);
    __HSM_TRACE_DEBUG__("----> AsyncHsm::onEnter: done");

    return true;
}

void AsyncHsm::onStateChanged(const VariantList_t& args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onStateChanged");
    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);

    __HSM_TRACE_DEBUG__("----> notify_one");
    mSyncVariableCheck = true;
    mSyncVariable.notify_one();

    __HSM_TRACE_DEBUG__("----> AsyncHsm::onStateChanged: wait");
    mBlockNextStep.wait(lck);
    __HSM_TRACE_DEBUG__("----> AsyncHsm::onStateChanged: done");
}

void AsyncHsm::onNextStateTransition(const VariantList_t& args)
{
    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);

    __HSM_TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onNextStateTransition");
    mSyncVariableCheck = true;
    mSyncVariable.notify_one();

    mBlockNextStep.wait(lck);
}

bool AsyncHsm::waitAsyncOperation(const int timeoutMs)
{
    std::unique_lock<std::mutex> lck(mSyncLock);

    __HSM_TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::waitAsyncOperation");
    bool res = mSyncVariable.wait_for(lck, std::chrono::milliseconds(timeoutMs), [&](){ return mSyncVariableCheck.load(); });
    mSyncVariableCheck = false;
    __HSM_TRACE_DEBUG__("----> AsyncHsm::waitAsyncOperation: DONE (res=%s)", BOOL2STR(res));

    return true;
}

void AsyncHsm::unblockNextStep()
{
    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);

    __HSM_TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::unblockNextStep");
    mBlockNextStep.notify_one();
}