// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, history_simple)
{
    TEST_DESCRIPTION("simple history test; shallow, one level");
    // *F -> P1 {*A, B, H[x]} -> C -> H

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerHistory(AbcState::P1, AbcState::H);

    registerTransition(AbcState::F, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::H, AbcEvent::E3);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));
}

TEST_F(ABCHsm, history_default_shallow)
{
    TEST_DESCRIPTION("if transitioning to SHALLOW history state before it was able to store any "
                     "previous states, HSM should use history default target");
    // F -> P1 { *A, P2{ B }, H[-P2] }

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::P2);
    registerSubstateEntryPoint(AbcState::P2, AbcState::B);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::SHALLOW, AbcState::P2);

    registerTransition(AbcState::F, AbcState::H, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    // default history target should be used
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::B}));
}

TEST_F(ABCHsm, history_default_deep)
{
    TEST_DESCRIPTION("if transitioning to DEEP history state before it was able to store any "
                     "previous states, HSM should use history default target");
    // F -> P1 { *A, P2{ B }, H[-P2] }

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::P2);
    registerSubstateEntryPoint(AbcState::P2, AbcState::B);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::DEEP, AbcState::P2);

    registerTransition(AbcState::F, AbcState::H, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    // default history target should be used
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::B}));
}

TEST_F(ABCHsm, history_no_default)
{
    TEST_DESCRIPTION("if there is no default transition defined for history "
                     "entry it should transition to parent's entry point");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerHistory(AbcState::P1, AbcState::H);

    registerTransition(AbcState::F, AbcState::H, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    // parent's entry point should be used
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));
}

TEST_F(ABCHsm, history_deep)
{
    TEST_DESCRIPTION("when deep history is used, HSM should activate "
                     "exact states which were active before exiting parent");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerSubstateEntryPoint(AbcState::P1, AbcState::P2);
    registerSubstateEntryPoint(AbcState::P2, AbcState::P3);
    registerSubstateEntryPoint(AbcState::P3, AbcState::A);
    registerSubstate(AbcState::P3, AbcState::B);
    registerSubstate(AbcState::P3, AbcState::D);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::DEEP);

    registerTransition(AbcState::F, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::D, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::H, AbcEvent::E3);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));
    // TODO: restoring multiple states is not done synchronously

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B, AbcState::D}));
}

TEST_F(ABCHsm, history_shallow)
{
    TEST_DESCRIPTION("when shallow history is used, HSM should activate "
                     "only direct child of the parent which owns history state");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerSubstateEntryPoint(AbcState::P1, AbcState::P2);
    registerSubstateEntryPoint(AbcState::P2, AbcState::A);
    registerSubstate(AbcState::P2, AbcState::B);
    registerSubstate(AbcState::P2, AbcState::D);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::SHALLOW);

    registerTransition(AbcState::F, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::D, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::H, AbcEvent::E3);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::A}));
}

TEST_F(ABCHsm, history_multiple)
{
    TEST_DESCRIPTION("history should be saved correctly when exiting from a top "
                     "level parent which contains history and substate with history");
    // TODO: review is this is the best way to validate this scenario
    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerSubstateEntryPoint(AbcState::P1, AbcState::P2);
    registerSubstateEntryPoint(AbcState::P2, AbcState::P3);
    registerSubstateEntryPoint(AbcState::P3, AbcState::A);
    registerSubstate(AbcState::P3, AbcState::B);
    registerSubstate(AbcState::P3, AbcState::D);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::DEEP);
    registerHistory(AbcState::P2, AbcState::H2, ABCHsm::HistoryType::SHALLOW);

    registerTransition(AbcState::F, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::D, AbcEvent::E1);

    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::H, AbcEvent::E2);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));



    //-------------------------------------------
    // VALIDATION
    // history H is activated
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B, AbcState::D}));
}

TEST_F(ABCHsm, history_callbacks)
{
    TEST_DESCRIPTION("check that all callbacks are correctly called during history transition");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD, &ABCHsm::onDEnter, &ABCHsm::onDExit);
    registerState<ABCHsm>(AbcState::P1, this, &ABCHsm::onP1, &ABCHsm::onP1Enter, &ABCHsm::onP1Exit);
    registerState<ABCHsm>(AbcState::P2, this, &ABCHsm::onP2, &ABCHsm::onP2Enter, &ABCHsm::onP2Exit);
    registerState<ABCHsm>(AbcState::P3, this, &ABCHsm::onP3, &ABCHsm::onP3Enter, &ABCHsm::onP3Exit);

    registerSubstateEntryPoint(AbcState::P1, AbcState::P2);
    registerSubstateEntryPoint(AbcState::P2, AbcState::P3);
    registerSubstateEntryPoint(AbcState::P3, AbcState::A);
    registerSubstate(AbcState::P3, AbcState::B);
    registerSubstate(AbcState::P3, AbcState::D);
    registerHistory<ABCHsm>(AbcState::P1, AbcState::H, ABCHsm::HistoryType::DEEP, AbcState::P2,
                            this, &ABCHsm::onRestoreHistoryTransition);

    registerTransition(AbcState::F, AbcState::H, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::D, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::H, AbcEvent::E3, this, &ABCHsm::onE3Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::A}));
    ASSERT_EQ(mTransitionCounterRestoreHistory, 1);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B, AbcState::D}));
    EXPECT_EQ(mTransitionCounterRestoreHistory, 1);
    EXPECT_EQ(mTransitionCounterE3, 1);

    EXPECT_EQ(mStateCounterP1, 2);
    EXPECT_EQ(mStateCounterP1Enter, 2);
    EXPECT_EQ(mStateCounterP1Exit, 1);
    EXPECT_EQ(mStateCounterP2, 2);
    EXPECT_EQ(mStateCounterP2Enter, 2);
    EXPECT_EQ(mStateCounterP2Exit, 1);
    EXPECT_EQ(mStateCounterP3, 2);
    EXPECT_EQ(mStateCounterP3Enter, 2);
    EXPECT_EQ(mStateCounterP3Exit, 1);

    EXPECT_EQ(mStateCounterA, 1);
    EXPECT_EQ(mStateCounterAEnter, 1);
    EXPECT_EQ(mStateCounterAExit, 1);
    EXPECT_EQ(mStateCounterB, 2);
    EXPECT_EQ(mStateCounterBEnter, 2);
    EXPECT_EQ(mStateCounterBExit, 1);
    EXPECT_EQ(mStateCounterD, 2);
    EXPECT_EQ(mStateCounterDEnter, 2);
    EXPECT_EQ(mStateCounterDExit, 1);
}

TEST_F(ABCHsm, history_parallel)
{
    TEST_DESCRIPTION("history should restore parallel states");
    // *F -> P1 { A -> B + C, H } -> D

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    // two entry points to activate in parallel
    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::C);
    registerHistory(AbcState::P1, AbcState::H);

    registerTransition(AbcState::F, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::D, AbcEvent::E2);
    registerTransition(AbcState::D, AbcState::H, AbcEvent::E3);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));
}