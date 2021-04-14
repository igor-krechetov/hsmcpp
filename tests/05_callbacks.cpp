#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, callbacks_class_pointers)
{
    TEST_DESCRIPTION("test that all collbacks work when specified as class members");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, nullptr, nullptr, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, nullptr);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1, this, &ABCHsm::onE1Transition, &ABCHsm::conditionTrue);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY, "test", 7));

    //-------------------------------------------
    // VALIDATION
    VariantList_t expectedArgs;

    expectedArgs.push_back(Variant::make("test"));
    expectedArgs.push_back(Variant::make(7));

    EXPECT_EQ(getLastActiveState(), AbcState::B);
    EXPECT_EQ(mStateCounterAExit, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mTransitionCounterE1, 1);
    EXPECT_EQ(mConditionTrueCounter, 1);

    EXPECT_EQ(mArgsB, expectedArgs);
    EXPECT_EQ(mArgsBEnter, expectedArgs);
    EXPECT_EQ(mTransitionArgsE1, expectedArgs);
    EXPECT_EQ(mArgsConditionTrue, expectedArgs);
}

TEST_F(ABCHsm, callbacks_lambdas)
{
    TEST_DESCRIPTION("test that all collbacks work when specified as lambda functions");

    //-------------------------------------------
    // PRECONDITIONS
    int stateCounterAExit = 0;
    int stateCounterBEnter = 0;
    int stateCounterB = 0;
    int transitionCounterE1 = 0;
    int conditionTrueCounter = 0;
    VariantList_t argsB;
    VariantList_t argsBEnter;
    VariantList_t transitionArgsE1;
    VariantList_t argsConditionTrue;

    registerState(AbcState::A,
                  nullptr,
                  nullptr,
                  [&](){ ++stateCounterAExit; return true; });
    registerState(AbcState::B,
                  [&](const VariantList_t& args){ ++stateCounterB; argsB = args; },
                  [&](const VariantList_t& args){ ++stateCounterBEnter; argsBEnter = args; return true; },
                  nullptr);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1,
                       [&](const VariantList_t& args){ ++transitionCounterE1; transitionArgsE1 = args; return true; },
                       [&](const VariantList_t& args){ ++conditionTrueCounter; argsConditionTrue = args; return true; });

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY, "test", 7));

    //-------------------------------------------
    // VALIDATION
    VariantList_t expectedArgs;

    expectedArgs.push_back(Variant::make("test"));
    expectedArgs.push_back(Variant::make(7));

    EXPECT_EQ(getLastActiveState(), AbcState::B);
    EXPECT_EQ(stateCounterAExit, 1);
    EXPECT_EQ(stateCounterBEnter, 1);
    EXPECT_EQ(stateCounterB, 1);
    EXPECT_EQ(transitionCounterE1, 1);
    EXPECT_EQ(conditionTrueCounter, 1);

    EXPECT_EQ(argsB, expectedArgs);
    EXPECT_EQ(argsBEnter, expectedArgs);
    EXPECT_EQ(transitionArgsE1, expectedArgs);
    EXPECT_EQ(argsConditionTrue, expectedArgs);
}