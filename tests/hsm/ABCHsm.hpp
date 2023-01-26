// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_ABCHSM_HPP
#define HSMCPP_TESTS_HSM_ABCHSM_HPP

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS                         "ABCHsm"

enum class AbcState
{
    A, B, C, D, E, F,
    H, H2,
    P1, P2, P3, P4,
    F1, F2, F3,
};

enum class AbcEvent
{
    E1, E2, E3,
    EXIT1, EXIT2,

    INVALID = INVALID_ID

};

class ABCHsm: public testing::Test
            , public HierarchicalStateMachine<AbcState, AbcEvent>
{
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

    inline void initializeHsm()
    {
        INITIALIZE_HSM();
    }

    std::string getStateName(const AbcState state) const override;
    std::string getEventName(const AbcEvent event) const override;

protected:
    void SetUp() override;
    void TearDown() override;

protected:
    int mConditionTrueCounter = 0;
    VariantVector_t mArgsConditionTrue;
};

#endif // HSMCPP_TESTS_HSM_ABCHSM_HPP