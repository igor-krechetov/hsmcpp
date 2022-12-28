// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/TrafficLightHsm.hpp"
#include "hsm/ABCHsm.hpp"
#include "hsm/AsyncHsm.hpp"

TEST_F(ABCHsm, substate_entrypoint)
{
    TEST_DESCRIPTION("");
    // A -e1-> P1 { *B, C }

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));
}

TEST_F(ABCHsm, substate_multiple_entrypoints_conditional)
{
    TEST_DESCRIPTION("hsm must support multiple exclusive conditional entry points");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B, AbcEvent::E1) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E2);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E3);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_multiple_entrypoints_default)
{
    TEST_DESCRIPTION("if state contains multiple entry points that are both conditional and non-conditional then "
                     "conditional ones will have priority");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E2);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E3);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_multiple_entrypoints_conditions)
{
    TEST_DESCRIPTION("entry point transitions can have condition callbacks defined");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B, AbcEvent::E1, [](const hsmcpp::VariantVector_t&){ return true; }, false) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E1, [](const hsmcpp::VariantVector_t&){ return false; }, false) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));
}

TEST_F(ABCHsm, substate_block_conditional_entry_transition)
{
    TEST_DESCRIPTION("if state doesnt have matching entry points for ongoing transition, then transition will be canceled");
    // *A -e1/e2-> P1{e2->*P2{e1->*B}, e1->*P3{e1->*C}}
    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::P2, AbcEvent::E2) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P2, AbcState::B, AbcEvent::E1) );

    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::P3, AbcEvent::E1) );
    ASSERT_TRUE( registerSubstateEntryPoint(AbcState::P3, AbcState::C, AbcEvent::E1) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    // NOTE: transition should be blocked because P2's entry point is defined with E1 event
    ASSERT_FALSE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P3, AbcState::C}));

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_entrypoint_substate)
{
    TEST_DESCRIPTION("entry point of a substate could be another state with substates");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::P2) );
    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P2, AbcState::P3) );
    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P3, AbcState::B) );
    EXPECT_TRUE( registerSubstate(AbcState::P3, AbcState::C) );

    initializeHsm();

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B}));
    EXPECT_EQ(mStateCounterA, 0);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterC, 0);
}

TEST_F(TrafficLightHsm, substate_exit_single)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::STARTING}));
    ASSERT_TRUE(transitionSync(TrafficLightEvent::NEXT_STATE, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::OPERABLE, TrafficLightState::RED}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_OFF, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::OFF}));
    EXPECT_EQ(mStateCounterStarting, 1);
    EXPECT_EQ(mStateCounterRed, 1);
    EXPECT_EQ(mStateCounterOff, 1);
    EXPECT_EQ(mTransitionCounterTurnOff, 1);
}

TEST_F(ABCHsm, substate_exit_multiple_layers)
{
    TEST_DESCRIPTION("Validate that exiting from multiple depth states on a top level transition is correctly handled");
    // *A -e1-> P1{ *B, P2{*C, P3{*D}} }

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::B) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::P2) );
    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P2, AbcState::C) );
    EXPECT_TRUE( registerSubstate(AbcState::P2, AbcState::P3) );
    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P3, AbcState::D) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::P2, AbcEvent::E1);
    registerTransition(AbcState::C, AbcState::P3, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E2);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    EXPECT_EQ(mStateCounterA, 1);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterD, 1);
}

TEST_F(ABCHsm, substate_safe_registration)
{
    TEST_DESCRIPTION("If HSM is compiled with safety check then it should prevent cyclic and multiple inclusions of substates. "
                     "This test will fail if HSM_ENABLE_SAFE_STRUCTURE is not defined");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P1, AbcState::A) );

    //-------------------------------------------
    // ACTIONS

    // each state can be a part of only one substate
    EXPECT_FALSE( registerSubstateEntryPoint(AbcState::P2, AbcState::A) );// A is already part of P1

    // prevent recursion
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::B) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::P2) );
    EXPECT_FALSE( registerSubstateEntryPoint(AbcState::P2, AbcState::P1) );// P1 -> B2 -X P1

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P4, AbcState::C) );
    EXPECT_TRUE( registerSubstate(AbcState::P4, AbcState::P3) );
    EXPECT_FALSE( registerSubstateEntryPoint(AbcState::P3, AbcState::P4) );// P3 -X P4 -> P3

    EXPECT_TRUE( registerSubstateEntryPoint(AbcState::P3, AbcState::P1) );
    EXPECT_FALSE( registerSubstateEntryPoint(AbcState::P2, AbcState::P4) );// prevent recursion: (P4) -> P3 -> P1 -> P2 -X (P4)

    initializeHsm();

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_error_no_entrypoint)
{
    TEST_DESCRIPTION("transition to a state should fail if no entry point was defined");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::B) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

TEST_F(AsyncHsm, substate_parent_as_initial)
{
    TEST_DESCRIPTION("when parent state is set as initial it should automatically transition into substate on startup");

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AsyncHsmState::P1);
    registerState<AsyncHsm>(AsyncHsmState::B, this, &AsyncHsm::onStateChanged);
    ASSERT_TRUE(registerSubstateEntryPoint(AsyncHsmState::P1, AsyncHsmState::P2));
    ASSERT_TRUE(registerSubstateEntryPoint(AsyncHsmState::P2, AsyncHsmState::B));

    //-------------------------------------------
    // ACTIONS
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation(200));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AsyncHsmState::P1, AsyncHsmState::P2, AsyncHsmState::B}));
}