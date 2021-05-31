#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, history_simple)
{
    TEST_DESCRIPTION("simple history test; shallow, one level");

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

    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::B);
}

TEST_F(ABCHsm, history_default)
{
    TEST_DESCRIPTION("if transitioning to history state before it was able to store any "
                     "previous states HSM should sould use history default target");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::D);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::SHALLOW, AbcState::D);

    registerTransition(AbcState::F, AbcState::H, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::D, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::H, AbcEvent::E3);

    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // default history target should be used
    ASSERT_EQ(getLastActiveState(), AbcState::D);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::B);
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
    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    // parent's entry point should be used
    EXPECT_EQ(getLastActiveState(), AbcState::A);
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
    registerSubstateEntryPoint(AbcState::P2, AbcState::A);
    registerSubstate(AbcState::P2, AbcState::B);
    registerSubstate(AbcState::P2, AbcState::D);
    registerHistory(AbcState::P1, AbcState::H, ABCHsm::HistoryType::DEEP);

    registerTransition(AbcState::F, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::D, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::H, AbcEvent::E3);

    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));
    // TODO: restoring multiple states is not done in synchronously

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::D}));
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
    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::A);
}

TEST_F(ABCHsm, history_multiple)
{
    TEST_DESCRIPTION("history should be saved correctly when exiting from a top "
                     "level parent which contains history and substate with history");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);
    registerState<ABCHsm>(AbcState::E, this, &ABCHsm::onE);

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
    registerTransition(AbcState::P1, AbcState::E, AbcEvent::E3);
    // NOTE: this is technically possible, but not correct from HSM design point of view.
    //       doing it only to simplify test
    registerTransition(AbcState::E, AbcState::H2, AbcEvent::E3);

    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    // history H is activated
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::E);

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    // history H2 is activated
    ASSERT_EQ(getLastActiveState(), AbcState::A);
}

TEST_F(ABCHsm, history_callbacks)
{
    TEST_DESCRIPTION("check that all callbacks are correctly called during history transition");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::F);

    registerState<ABCHsm>(AbcState::F);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::D);
    registerHistory<ABCHsm>(AbcState::P1,
                            AbcState::H,
                            ABCHsm::HistoryType::SHALLOW,
                            AbcState::D,
                            this,
                            &ABCHsm::onRestoreHistoryTransition);

    registerTransition(AbcState::F, AbcState::H, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::D, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::H, AbcEvent::E3, this, &ABCHsm::onE3Transition);

    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::F);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::D);
    EXPECT_EQ(mTransitionCounterRestoreHistory, 1);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::B);
    EXPECT_EQ(mTransitionCounterRestoreHistory, 2);
    EXPECT_EQ(mTransitionCounterE3, 1);
    EXPECT_EQ(mStateCounterB, 2);
    EXPECT_EQ(mStateCounterBEnter, 2);
    EXPECT_EQ(mStateCounterBExit, 1);
    EXPECT_EQ(mStateCounterD, 1);
}
