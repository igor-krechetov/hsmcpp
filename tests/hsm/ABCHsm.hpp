#ifndef __HSMCPP_TESTS_HSM_ABCHSM_HPP__
#define __HSMCPP_TESTS_HSM_ABCHSM_HPP__

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "ABCHsm"

enum class AbcState
{
    A, B, C, D, E, F,
    H, H2,
    P1, P2, P3, P4,
    F1, F2, F3
};

enum class AbcEvent
{
    E1, E2, E3,
    EXIT1, EXIT2
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

protected:
    void SetUp() override;
    void TearDown() override;

protected:
    int mConditionTrueCounter = 0;
    VariantVector_t mArgsConditionTrue;
};

#endif // __HSMCPP_TESTS_HSM_ABCHSM_HPP__