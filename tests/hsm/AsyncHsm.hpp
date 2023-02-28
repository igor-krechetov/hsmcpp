// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_ASYNCHSM_HPP
#define HSMCPP_TESTS_HSM_ASYNCHSM_HPP

#include <atomic>

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"
#include "hsmcpp/os/ConditionVariable.hpp"
#include "hsmcpp/os/Mutex.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "AsyncHsm"

namespace AsyncHsmState {
    const hsmcpp::StateID_t A = 0;
    const hsmcpp::StateID_t B = 1;
    const hsmcpp::StateID_t C = 2;
    const hsmcpp::StateID_t D = 3;
    const hsmcpp::StateID_t E = 4;

    const hsmcpp::StateID_t P1 = 5;
    const hsmcpp::StateID_t P2 = 6;
    const hsmcpp::StateID_t P3 = 7;
}

namespace AsyncHsmEvent {
    const hsmcpp::EventID_t NEXT_STATE = 0;
    const hsmcpp::EventID_t EXIT_SUBSTATE = 1;
};

class AsyncHsm : public testing::Test, public HierarchicalStateMachine {
public:
    AsyncHsm();
    virtual ~AsyncHsm();

    void SetUp() override;
    void TearDown() override;

    inline void initializeHsm() {
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

#endif  // HSMCPP_TESTS_HSM_ASYNCHSM_HPP