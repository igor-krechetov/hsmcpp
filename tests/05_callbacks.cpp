// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, callbacks_class_pointers)
{
    TEST_DESCRIPTION("test that all callbacks work when specified as class members");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, nullptr, nullptr, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, nullptr);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1, this, &ABCHsm::onE1Transition, &ABCHsm::conditionTrue);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY, "test", 7));

    //-------------------------------------------
    // VALIDATION
    VariantVector_t expectedArgs;

    expectedArgs.push_back(Variant::make("test"));
    expectedArgs.push_back(Variant::make(7));

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
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
    TEST_DESCRIPTION("test that all callbacks work when specified as lambda functions");

    //-------------------------------------------
    // PRECONDITIONS
    int stateCounterAExit = 0;
    int stateCounterBEnter = 0;
    int stateCounterB = 0;
    int transitionCounterE1 = 0;
    int conditionTrueCounter = 0;
    VariantVector_t argsB;
    VariantVector_t argsBEnter;
    VariantVector_t transitionArgsE1;
    VariantVector_t argsConditionTrue;

    registerState(AbcState::A,
                  nullptr,
                  nullptr,
                  [&](){ ++stateCounterAExit; return true; });
    registerState(AbcState::B,
                  [&](const VariantVector_t& args){ ++stateCounterB; argsB = args; },
                  [&](const VariantVector_t& args){ ++stateCounterBEnter; argsBEnter = args; return true; },
                  nullptr);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1,
                       [&](const VariantVector_t& args){ ++transitionCounterE1; transitionArgsE1 = args; return true; },
                       [&](const VariantVector_t& args){ ++conditionTrueCounter; argsConditionTrue = args; return true; });

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY, "test", 7));

    //-------------------------------------------
    // VALIDATION
    VariantVector_t expectedArgs;

    expectedArgs.push_back(Variant::make("test"));
    expectedArgs.push_back(Variant::make(7));

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
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

TEST_F(ABCHsm, callbacks_entering_substates)
{
    TEST_DESCRIPTION("when entering a state with substates, HSM should only call it's onEnter, onState callbacks");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::A);

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::P1, this, &ABCHsm::onP1, &ABCHsm::onP1Enter, &ABCHsm::onP1Exit);

    registerSubstateEntryPoint(AbcState::P1, AbcState::B);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    EXPECT_EQ(mStateCounterP1, 1);
    EXPECT_EQ(mStateCounterP1Enter, 1);
    EXPECT_EQ(mStateCounterP1Exit, 0);
}


TEST_F(ABCHsm, callbacks_exiting_substates)
{
    TEST_DESCRIPTION("when exiting a state with substates, HSM should only call it's onExit callbacks");
    // *A -e1-> P1{*B} -e2-> A

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::A);

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::P1, this, &ABCHsm::onP1, &ABCHsm::onP1Enter, &ABCHsm::onP1Exit);

    registerSubstateEntryPoint(AbcState::P1, AbcState::B);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E2);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    ASSERT_EQ(mStateCounterP1, 1);
    ASSERT_EQ(mStateCounterP1Enter, 1);
    ASSERT_EQ(mStateCounterP1Exit, 0);
    ASSERT_EQ(mStateCounterB, 1);
    ASSERT_EQ(mStateCounterBEnter, 1);
    ASSERT_EQ(mStateCounterBExit, 0);

    //-------------------------------------------
    // ACTIONS
    mStateCounterP1Exit = 0;
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExit, 1);

    EXPECT_EQ(mStateCounterP1, 1);
    EXPECT_EQ(mStateCounterP1Enter, 1);
    EXPECT_EQ(mStateCounterP1Exit, 1);
}

// TODO: calling callbacks of initial state