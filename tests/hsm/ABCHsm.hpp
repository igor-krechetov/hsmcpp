#ifndef __HSMCPP_TESTS_HSM_ABCHSM_HPP__
#define __HSMCPP_TESTS_HSM_ABCHSM_HPP__

#include "TestsCommon.hpp"
#include "hsm.hpp"

enum class AbcState
{
    A, B, C,
    P1, P2, P3, P4
};

enum class AbcEvent
{
    E1, E2, E3
};

#define GEN_STATE_ACTION(_name)         void on##_name(const VariantList_t& args){ printf("----> " #_name "\n"); }

class ABCHsm: public testing::Test
            , public HierarchicalStateMachine<AbcState, AbcEvent, ABCHsm>
{
public:
    ABCHsm();
    virtual ~ABCHsm();

    GEN_STATE_ACTION(A)
    GEN_STATE_ACTION(B)
    GEN_STATE_ACTION(C)

protected:
    void SetUp() override;
    void TearDown() override;
};

#endif // __HSMCPP_TESTS_HSM_ABCHSM_HPP__