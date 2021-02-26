#include "AsyncHsm.hpp"
#include "logging.hpp"
#include <chrono>

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "AsyncHsm"

AsyncHsm::AsyncHsm() : HierarchicalStateMachine(AsyncHsmState::A)
{
}

AsyncHsm::~AsyncHsm()
{}

void AsyncHsm::SetUp()
{
}

void AsyncHsm::TearDown()
{
    __TRACE_CALL_DEBUG_ARGS__("----> TearDown AsyncHsm");
    mSyncVariable.notify_all();
    mBlockNextStep.notify_all();
}

bool AsyncHsm::onExit()
{
    __TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onExit");
    mSyncVariable.notify_one();

    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);
    __TRACE_DEBUG__("----> AsyncHsm::onExit: wait");
    mBlockNextStep.wait(lck);
    __TRACE_DEBUG__("----> AsyncHsm::onExit: done");

    return true;
}

bool AsyncHsm::onEnter(const VariantList_t& args)
{
    __TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onEnter");
    mSyncVariable.notify_one();

    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);
    __TRACE_DEBUG__("----> AsyncHsm::onEnter: wait");
    mBlockNextStep.wait(lck);
    __TRACE_DEBUG__("----> AsyncHsm::onEnter: done");

    return true;
}

void AsyncHsm::onStateChanged(const VariantList_t& args)
{
    __TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onStateChanged");
    mSyncVariable.notify_one();

    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);
    __TRACE_DEBUG__("----> AsyncHsm::onStateChanged: wait");
    mBlockNextStep.wait(lck);
    __TRACE_DEBUG__("----> AsyncHsm::onStateChanged: done");
}

void AsyncHsm::onNextStateTransition(const VariantList_t& args)
{
    __TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::onNextStateTransition");
    mSyncVariable.notify_one();

    std::unique_lock<std::mutex> lck(mSyncBlockNextStep);
    mBlockNextStep.wait(lck);
}

bool AsyncHsm::waitAsyncOperation(const int timeoutMs)
{
    std::unique_lock<std::mutex> lck(mSyncLock);

    __TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::waitAsyncOperation");
    mSyncVariable.wait_for(lck, std::chrono::milliseconds(timeoutMs));
    __TRACE_DEBUG__("----> AsyncHsm::waitAsyncOperation: DONE");
    // TODO: timeout

    return true;
}

void AsyncHsm::unblockNextStep()
{
    __TRACE_CALL_DEBUG_ARGS__("----> AsyncHsm::unblockNextStep");
    mBlockNextStep.notify_one();
}