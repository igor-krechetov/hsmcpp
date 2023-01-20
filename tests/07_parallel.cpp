// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"
#include "hsm/AsyncHsm.hpp"

// Notation
// B        : regular state
// ->       : transition
// B + C    : transition to 2 states at the same time
// *A       : initial active state
// [...]    : parent state
// #C       : entry point
// !#C      : entry point with a false condition
// {Cx}     : transition from state C was blocked

TEST_F(ABCHsm, parallel_transition_01)
{
    TEST_DESCRIPTION("*A -> B + C");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_02)
{
    TEST_DESCRIPTION("*A -> B + [#C]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::P1, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_05)
{
    TEST_DESCRIPTION("*A -> B + [C]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, parallel_transition_03)
{
    TEST_DESCRIPTION("*A -> B + [!#C, #D]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2) );
    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::D) );

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::P1, AbcState::D}));
}

TEST_F(AsyncHsm, parallel_transition_04)
{
    TEST_DESCRIPTION("[*A, B] -> C + D");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(AsyncHsmState::A);
    registerState(AsyncHsmState::B);
    registerState(AsyncHsmState::C);
    registerState(AsyncHsmState::D);

    ASSERT_TRUE( registerSubstateEntryPoint(AsyncHsmState::P1, AsyncHsmState::A) );
    ASSERT_TRUE( registerSubstate(AsyncHsmState::P1, AsyncHsmState::B) );

    registerTransition(AsyncHsmState::A, AsyncHsmState::B, AsyncHsmEvent::EXIT_SUBSTATE);
    registerTransition(AsyncHsmState::P1, AsyncHsmState::C, AsyncHsmEvent::NEXT_STATE);
    registerTransition(AsyncHsmState::P1, AsyncHsmState::D, AsyncHsmEvent::NEXT_STATE);

    setInitialState(AsyncHsmState::P1);
    initializeHsm();
    waitAsyncOperation(300);// wait for A state to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AsyncHsmState::P1, AsyncHsmState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AsyncHsmEvent::NEXT_STATE, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AsyncHsmState::C, AsyncHsmState::D}));
}


TEST_F(ABCHsm, parallel_transition_11)
{
    TEST_DESCRIPTION("A -> [#B, #C] -> D");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B) );
    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::D, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));
}

TEST_F(ABCHsm, parallel_transition_06)
{
    TEST_DESCRIPTION("*A -> B + [!#C]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2) );

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, parallel_transition_07)
{
    TEST_DESCRIPTION("*A -> B + [#[C]]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::P2) );
    EXPECT_TRUE( registerSubstate(AbcState::P2, AbcState::C) );

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, parallel_transition_08)
{
    TEST_DESCRIPTION("*A -> B + C -> A: check that transitions are not applied recursively");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);

    // since transitions are applied one by one we want to make sure that HSM will not transition to A while doing (*A -> B + C)
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);
    registerTransition(AbcState::C, AbcState::A, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_09)
{
    TEST_DESCRIPTION("A -> [*#B + *#C]>e2  -e2-> D");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD, &ABCHsm::onDEnter, &ABCHsm::onDExit);

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::E2, this, &ABCHsm::onE2Transition);
    registerSelfTransition<ABCHsm>(AbcState::P1, AbcEvent::E2, ABCHsm::TransitionType::INTERNAL_TRANSITION, this, &ABCHsm::onSelfTransition);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExit, 1);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);
    EXPECT_EQ(mStateCounterD, 1);
    EXPECT_EQ(mStateCounterDEnter, 1);
    EXPECT_EQ(mStateCounterDExit, 0);

    EXPECT_EQ(mTransitionCounterE2, 1);
    EXPECT_EQ(mTransitionCounterSelf, 1);
}

TEST_F(AsyncHsm, parallel_transition_10_internal_priority)
{
    TEST_DESCRIPTION("[*A -e1-> B + C] -e1-> D: internal transitions have priority over external ones");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AsyncHsmState::P1);
    registerState<AsyncHsm>(AsyncHsmState::A);
    registerState<AsyncHsm>(AsyncHsmState::B);
    registerState<AsyncHsm>(AsyncHsmState::C);
    registerState<AsyncHsm>(AsyncHsmState::D);

    EXPECT_TRUE( registerSubstateEntryPoint(AsyncHsmState::P1, AsyncHsmState::A) );
    EXPECT_TRUE( registerSubstate(AsyncHsmState::P1, AsyncHsmState::B) );
    EXPECT_TRUE( registerSubstate(AsyncHsmState::P1, AsyncHsmState::C) );

    registerTransition(AsyncHsmState::A, AsyncHsmState::B, AsyncHsmEvent::NEXT_STATE);
    registerTransition(AsyncHsmState::A, AsyncHsmState::C, AsyncHsmEvent::NEXT_STATE);
    registerTransition(AsyncHsmState::P1, AsyncHsmState::D, AsyncHsmEvent::NEXT_STATE);

    initializeHsm();
    waitAsyncOperation(300);// wait for A state to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AsyncHsmState::P1, AsyncHsmState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AsyncHsmEvent::NEXT_STATE, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AsyncHsmState::P1, AsyncHsmState::B, AsyncHsmState::C}));
}

TEST_F(ABCHsm, parallel_transition_canceled_01)
{
    TEST_DESCRIPTION("A -> [*#B + *#C] -> D {Cx}");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExitCancel);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D);

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C) );


    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::E2, this, &ABCHsm::onE2Transition);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExitCancel, 1);
    EXPECT_EQ(mStateCounterC, 2);
    EXPECT_EQ(mStateCounterCEnter, 2);
    EXPECT_EQ(mStateCounterCExit, 1);

    EXPECT_EQ(mTransitionCounterE2, 0);
}

TEST_F(ABCHsm, parallel_transition_canceled_02)
{
    TEST_DESCRIPTION("A -> / *B -> D + xE / *C -> F /");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD, &ABCHsm::onDEnter, &ABCHsm::onDExit);
    registerState<ABCHsm>(AbcState::E, this, &ABCHsm::onE, &ABCHsm::onEEnterCancel, &ABCHsm::onEExit);
    registerState<ABCHsm>(AbcState::F, this, &ABCHsm::onF, &ABCHsm::onFEnter, &ABCHsm::onFExit);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);

    registerTransition(AbcState::B, AbcState::D, AbcEvent::E2);
    registerTransition(AbcState::B, AbcState::E, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::F, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::D, AbcState::F, AbcState::B}));
    EXPECT_EQ(mStateCounterB, 2);
    EXPECT_EQ(mStateCounterBEnter, 2);
    EXPECT_EQ(mStateCounterBExit, 1);

    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);

    EXPECT_EQ(mStateCounterD, 1);
    EXPECT_EQ(mStateCounterDEnter, 1);
    EXPECT_EQ(mStateCounterDExit, 0);

    EXPECT_EQ(mStateCounterE, 0);
    EXPECT_EQ(mStateCounterEEnterCancel, 1);
    EXPECT_EQ(mStateCounterEExit, 0);

    EXPECT_EQ(mStateCounterF, 1);
    EXPECT_EQ(mStateCounterFEnter, 1);
    EXPECT_EQ(mStateCounterFExit, 0);
}

TEST_F(ABCHsm, parallel_transition_mult2one_01)
{
    TEST_DESCRIPTION("A -> *B + *C -> A");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::A, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

TEST_F(ABCHsm, parallel_transition_mult2one_02)
{
    TEST_DESCRIPTION("A -> *B + [*C] -> A");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E2);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::P1, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

TEST_F(ABCHsm, parallel_callbacks)
{
    TEST_DESCRIPTION("*A -> B + C -> A: check that all callbacks are correctly executed");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1, this, &ABCHsm::onE1Transition);
    registerTransition<ABCHsm>(AbcState::A, AbcState::C, AbcEvent::E1, this, &ABCHsm::onE1Transition);
    registerTransition<ABCHsm>(AbcState::B, AbcState::A, AbcEvent::E2, this, &ABCHsm::onE2Transition);
    registerTransition<ABCHsm>(AbcState::C, AbcState::A, AbcEvent::E2, this, &ABCHsm::onE2Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
    ASSERT_EQ(mStateCounterA, 0);
    ASSERT_EQ(mStateCounterAEnter, 0);
    ASSERT_EQ(mStateCounterAExit, 1);
    ASSERT_EQ(mTransitionCounterE1, 2);

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mStateCounterA, 1);
    EXPECT_EQ(mStateCounterAEnter, 1);
    EXPECT_EQ(mStateCounterAExit, 1);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExit, 1);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);

    ASSERT_EQ(mTransitionCounterE1, 2);
    ASSERT_EQ(mTransitionCounterE2, 2);
}

TEST_F(ABCHsm, parallel_selftransition)
{
    TEST_DESCRIPTION("*A -> A + B: when we have both a regular and self-transition self-transition will"
                     " be excecuted first before exiting state");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);

    registerSelfTransition<ABCHsm>(AbcState::A, AbcEvent::E1, ABCHsm::TransitionType::INTERNAL_TRANSITION, this, &ABCHsm::onE1Transition);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1, this, &ABCHsm::onE1Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mStateCounterA, 0);
    EXPECT_EQ(mStateCounterAEnter, 0);
    EXPECT_EQ(mStateCounterAExit, 1);
    EXPECT_EQ(mTransitionCounterE1, 2);
}

TEST_F(ABCHsm, parallel_selftransition_multiple)
{
    TEST_DESCRIPTION("*A -> A + A: check that multiple self transitions are correctly handled");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);

    registerSelfTransition<ABCHsm>(AbcState::A, AbcEvent::E1, ABCHsm::TransitionType::INTERNAL_TRANSITION, this, &ABCHsm::onE1Transition);
    registerSelfTransition<ABCHsm>(AbcState::A, AbcEvent::E1, ABCHsm::TransitionType::INTERNAL_TRANSITION, this, &ABCHsm::onE2Transition);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E3, this, &ABCHsm::onE3Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mStateCounterA, 0);
    EXPECT_EQ(mStateCounterAEnter, 0);
    EXPECT_EQ(mStateCounterAExit, 0);
    EXPECT_EQ(mTransitionCounterE1, 1);
    EXPECT_EQ(mTransitionCounterE2, 1);
    EXPECT_EQ(mTransitionCounterE3, 0);
}