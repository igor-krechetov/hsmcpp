// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_ABCHSM_HPP
#define HSMCPP_TESTS_HSM_ABCHSM_HPP

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "ABCHsm"

namespace AbcState {
    const hsmcpp::StateID_t A = 0;
    const hsmcpp::StateID_t B = 1;
    const hsmcpp::StateID_t C = 2;
    const hsmcpp::StateID_t D = 3;
    const hsmcpp::StateID_t E = 4;
    const hsmcpp::StateID_t F = 5;
    const hsmcpp::StateID_t H = 6;
    const hsmcpp::StateID_t H2 = 7;
    const hsmcpp::StateID_t P1 = 8;
    const hsmcpp::StateID_t P2 = 9;
    const hsmcpp::StateID_t P3 = 10;
    const hsmcpp::StateID_t P4 = 11;
    const hsmcpp::StateID_t F1 = 12;
    const hsmcpp::StateID_t F2 = 13;
    const hsmcpp::StateID_t F3 = 14;
}

namespace AbcEvent {
    const hsmcpp::EventID_t E1 = 0;
    const hsmcpp::EventID_t E2 = 1;
    const hsmcpp::EventID_t E3 = 2;
    const hsmcpp::EventID_t EXIT1 = 3;
    const hsmcpp::EventID_t EXIT2 = 4;

    const hsmcpp::EventID_t INVALID = INVALID_ID;
}

class ABCHsm : public testing::Test, public HierarchicalStateMachine {
public:
    ABCHsm();
    virtual ~ABCHsm();

    DEF_STATE_ACTION_IMPL(A)
    DEF_STATE_ACTION_IMPL(B)
    DEF_STATE_ACTION_IMPL(C)
    DEF_STATE_ACTION_IMPL(D)
    DEF_STATE_ACTION_IMPL(E)
    DEF_STATE_ACTION_IMPL(F)

    DEF_STATE_ACTION_IMPL(P1)
    DEF_STATE_ACTION_IMPL(P2)
    DEF_STATE_ACTION_IMPL(P3)

    DEF_STATE_ACTION_IMPL(H)
    DEF_TRANSITION_IMPL(RestoreHistory)

    DEF_TRANSITION_IMPL(E1)
    DEF_TRANSITION_IMPL(E2)
    DEF_TRANSITION_IMPL(E3)
    DEF_TRANSITION_IMPL(Self)

    bool conditionTrue(const VariantVector_t& args);

    inline void initializeHsm() {
        INITIALIZE_HSM();
    }

    std::string getStateName(const hsmcpp::StateID_t state) const override;
    std::string getEventName(const hsmcpp::EventID_t event) const override;

protected:
    void SetUp() override;
    void TearDown() override;

protected:
    int mConditionTrueCounter = 0;
    VariantVector_t mArgsConditionTrue;
};

#endif  // HSMCPP_TESTS_HSM_ABCHSM_HPP