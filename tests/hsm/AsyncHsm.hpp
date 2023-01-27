// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_ASYNCHSM_HPP
#define HSMCPP_TESTS_HSM_ASYNCHSM_HPP

#include "TestsCommon.hpp"
#include <atomic>

#include "hsmcpp/hsm.hpp"
#include "hsmcpp/os/Mutex.hpp"
#include "hsmcpp/os/ConditionVariable.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS                         "AsyncHsm"

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
    bool onEnter(const VariantVector_t& args);
    void onStateChanged(const VariantVector_t& args);
    void onNextStateTransition(const VariantVector_t& args);

    bool waitAsyncOperation(const int timeoutMs, const bool unblockNext);
    void unblockNextStep();

protected:
    hsmcpp::Mutex mSyncLock;
    hsmcpp::ConditionVariable mSyncVariable;
    std::atomic<bool> mSyncVariableCheck;

    hsmcpp::Mutex mSyncBlockNextStep;
    hsmcpp::ConditionVariable mBlockNextStep;
};

#endif // HSMCPP_TESTS_HSM_ASYNCHSM_HPP