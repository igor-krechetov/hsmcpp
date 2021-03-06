#ifndef __HSMCPP_TESTS_HSM_ASYNCHSM_HPP__
#define __HSMCPP_TESTS_HSM_ASYNCHSM_HPP__

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"
#include <mutex>
#include <atomic>
#include <condition_variable>

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "AsyncHsm"

enum class AsyncHsmState
{
    A, B, C, D, E,

    P1, P2, P3
};

enum class AsyncHsmEvent
{
    NEXT_STATE,
    EXIT_SUBSTATE
};

class AsyncHsm: public testing::Test
              , public HierarchicalStateMachine<AsyncHsmState, AsyncHsmEvent>
{
public:
    AsyncHsm();
    virtual ~AsyncHsm();

    void SetUp() override;
    void TearDown() override;

    inline void initializeHsm()
    {
        INITIALIZE_HSM();
    }

    bool onExit();
    bool onEnter(const VariantList_t& args);
    void onStateChanged(const VariantList_t& args);
    void onNextStateTransition(const VariantList_t& args);

    bool waitAsyncOperation(const int timeoutMs);
    void unblockNextStep();

protected:
    std::mutex mSyncLock;
    std::condition_variable mSyncVariable;
    std::atomic<bool> mSyncVariableCheck;

    std::mutex mSyncBlockNextStep;
    std::condition_variable mBlockNextStep;
};

#endif // __HSMCPP_TESTS_HSM_ASYNCHSM_HPP__