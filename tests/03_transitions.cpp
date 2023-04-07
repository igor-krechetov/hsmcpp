// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"
#include "hsm/TrafficLightHsm.hpp"

TEST_F(TrafficLightHsm, simple_transition) {
    TEST_DESCRIPTION("Simple transition between two states");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::STARTING);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 1);
}

TEST_F(TrafficLightHsm, transition_check) {
    TEST_DESCRIPTION("Checking that transition is possible before executing it");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    EXPECT_TRUE(isTransitionPossible(TrafficLightEvent::TURN_ON));
    EXPECT_FALSE(isTransitionPossible(TrafficLightEvent::TURN_OFF));
    EXPECT_FALSE(isTransitionPossible(TrafficLightEvent::NEXT_STATE));

    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_FALSE(isTransitionPossible(TrafficLightEvent::TURN_ON));
    EXPECT_FALSE(isTransitionPossible(TrafficLightEvent::TURN_OFF));
    EXPECT_TRUE(isTransitionPossible(TrafficLightEvent::NEXT_STATE));
}

TEST_F(TrafficLightHsm, transition_with_args) {
    TEST_DESCRIPTION("Test if args are correctly passed to transition action handler");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF);
    registerState(TrafficLightState::STARTING);
    registerTransition<TrafficLightHsm>(TrafficLightState::OFF,
                                        TrafficLightState::STARTING,
                                        TrafficLightEvent::TURN_ON,
                                        this,
                                        &TrafficLightHsm::onNextStateTransition);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, 12, "string", 12.75, false));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 1);

    ASSERT_EQ(mTransitionArgsNextState.size(), 4);

    EXPECT_TRUE(mTransitionArgsNextState[0].isNumeric());
    EXPECT_EQ(mTransitionArgsNextState[0].toInt64(), 12);

    EXPECT_TRUE(mTransitionArgsNextState[1].isString());
    EXPECT_STREQ(mTransitionArgsNextState[1].toString().c_str(), "string");

    EXPECT_TRUE(mTransitionArgsNextState[2].isNumeric());
    EXPECT_FLOAT_EQ(mTransitionArgsNextState[2].toDouble(), 12.75);

    EXPECT_TRUE(mTransitionArgsNextState[3].isBool());
    EXPECT_FALSE(mTransitionArgsNextState[3].toBool());
}

TEST_F(TrafficLightHsm, transition_failed_notification) {
    TEST_DESCRIPTION("Clients can receive notifications about failed transitions");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    hsmcpp::StateID_t prevState = getLastActiveState();

    ASSERT_FALSE(transitionSync(TrafficLightEvent::NEXT_STATE, TIMEOUT_SYNC_TRANSITION, 123));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mFailedTransitionCounter, 1);
    EXPECT_EQ(mLastFailedTransition, TrafficLightEvent::NEXT_STATE);

    ASSERT_EQ(mLastFailedTransitionArgs.size(), 1);
    EXPECT_TRUE(mLastFailedTransitionArgs[0].isNumeric());
    EXPECT_EQ(mLastFailedTransitionArgs[0].toInt64(), 123);
}

TEST_F(TrafficLightHsm, transition_non_existent) {
    TEST_DESCRIPTION("Check that non registered transitions don't change HSM state");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    hsmcpp::StateID_t prevState = getLastActiveState();

    ASSERT_FALSE(transitionSync(TrafficLightEvent::NEXT_STATE, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), prevState);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);

    EXPECT_EQ(mFailedTransitionCounter, 1);
    EXPECT_EQ(mLastFailedTransition, TrafficLightEvent::NEXT_STATE);
    EXPECT_TRUE(mLastFailedTransitionArgs.empty());
}

TEST_F(TrafficLightHsm, transition_cancel_on_exit) {
    TEST_DESCRIPTION("It should be possible to cancel transition if OnExit handler returns FALSE");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF,
                                   this,
                                   &TrafficLightHsm::onOff,
                                   nullptr,
                                   &TrafficLightHsm::onExitCancel);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterExitCancel, 1);
    EXPECT_EQ(mStateCounterExit, 0);
    EXPECT_EQ(mStateCounterEnter, 0);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);

    EXPECT_EQ(mFailedTransitionCounter, 1);
    EXPECT_EQ(mLastFailedTransition, TrafficLightEvent::TURN_ON);
    EXPECT_TRUE(mLastFailedTransitionArgs.empty());
}

TEST_F(TrafficLightHsm, transition_cancel_on_enter) {
    TEST_DESCRIPTION("It should be possible to cancel transition if onEnter handler returns FALSE");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF,
                                   this,
                                   &TrafficLightHsm::onOff,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING,
                                   this,
                                   &TrafficLightHsm::onStarting,
                                   &TrafficLightHsm::onEnterCancel);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION

    EXPECT_EQ(getLastActiveState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterExit, 1);
    EXPECT_EQ(mStateCounterEnter, 1);
    EXPECT_EQ(mStateCounterEnterCancel, 1);
    EXPECT_EQ(mStateCounterOff, 1);
    EXPECT_EQ(mStateCounterStarting, 0);

    EXPECT_EQ(mFailedTransitionCounter, 1);
    EXPECT_EQ(mLastFailedTransition, TrafficLightEvent::TURN_ON);
    EXPECT_TRUE(mLastFailedTransitionArgs.empty());
}

TEST_F(TrafficLightHsm, transition_self_internal) {
    TEST_DESCRIPTION(
        "Internal self transition should not trigger any state handlers. Only transition handler must be excecuted");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING,
                                   this,
                                   &TrafficLightHsm::onStarting,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);
    registerSelfTransition<TrafficLightHsm>(TrafficLightState::STARTING,
                                            TrafficLightEvent::NEXT_STATE,
                                            TrafficLightHsm::TransitionType::INTERNAL_TRANSITION,
                                            this,
                                            &TrafficLightHsm::onNextStateTransition);

    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::STARTING}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(TrafficLightEvent::NEXT_STATE, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::STARTING);
    EXPECT_EQ(mStateCounterExit, 0);
    EXPECT_EQ(mStateCounterEnter, 1);
    EXPECT_EQ(mStateCounterStarting, 1);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}

TEST_F(TrafficLightHsm, transition_self_external) {
    TEST_DESCRIPTION(
        "External self transition should result in exiting an reentering current state with all state callbacks correctly "
        "executed");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING,
                                   this,
                                   &TrafficLightHsm::onStarting,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);
    registerSelfTransition<TrafficLightHsm>(TrafficLightState::STARTING,
                                            TrafficLightEvent::NEXT_STATE,
                                            TrafficLightHsm::TransitionType::EXTERNAL_TRANSITION,
                                            this,
                                            &TrafficLightHsm::onNextStateTransition);

    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::STARTING}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(TrafficLightEvent::NEXT_STATE, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::STARTING);
    EXPECT_EQ(mStateCounterExit, 1);
    EXPECT_EQ(mStateCounterEnter, 2);
    EXPECT_EQ(mStateCounterStarting, 2);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}

TEST_F(ABCHsm, transition_self_external_deep) {
    TEST_DESCRIPTION("Correctly exit child subscates when doing external self transition on parent state");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::P1, this, &ABCHsm::onP1, &ABCHsm::onP1Enter, &ABCHsm::onP1Exit);
    registerState<ABCHsm>(AbcState::P2, this, &ABCHsm::onP2, &ABCHsm::onP2Enter, &ABCHsm::onP2Exit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::P2));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P2, AbcState::B));
    ASSERT_TRUE(registerSubstate(AbcState::P2, AbcState::C));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::C, AbcEvent::E1);
    registerSelfTransition<ABCHsm>(AbcState::P1,
                                   AbcEvent::E2,
                                   ABCHsm::TransitionType::EXTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onE2Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::B}));

    ASSERT_EQ(mStateCounterB, 1);
    ASSERT_EQ(mStateCounterBEnter, 1);
    ASSERT_EQ(mStateCounterBExit, 0);

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::C}));

    ASSERT_EQ(mStateCounterC, 1);
    ASSERT_EQ(mStateCounterCEnter, 1);
    ASSERT_EQ(mStateCounterCExit, 0);

    ASSERT_EQ(mStateCounterP1, 1);
    ASSERT_EQ(mStateCounterP1Enter, 1);
    ASSERT_EQ(mStateCounterP1Exit, 0);

    ASSERT_EQ(mStateCounterP2, 1);
    ASSERT_EQ(mStateCounterP2Enter, 1);
    ASSERT_EQ(mStateCounterP2Exit, 0);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    // after external self-transition HSM should exit C->P2->P1 and then enter P1->P2->B
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::B}));

    EXPECT_EQ(mTransitionCounterE2, 1);

    EXPECT_EQ(mStateCounterB, 2);
    EXPECT_EQ(mStateCounterBEnter, 2);
    EXPECT_EQ(mStateCounterBExit, 1);

    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);

    EXPECT_EQ(mStateCounterP1, 2);
    EXPECT_EQ(mStateCounterP1Enter, 2);
    EXPECT_EQ(mStateCounterP1Exit, 1);

    EXPECT_EQ(mStateCounterP2, 2);
    EXPECT_EQ(mStateCounterP2Enter, 2);
    EXPECT_EQ(mStateCounterP2Exit, 1);
}

TEST_F(TrafficLightHsm, transition_entrypoint_raicecondition) {
    TEST_DESCRIPTION("entrypoint transitions should be atomic and can't be canceled");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    hsmcpp::StateID_t prevState = getLastActiveState();

    ASSERT_FALSE(transitionSync(TrafficLightEvent::NEXT_STATE, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), prevState);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, transition_conditional_simple_true) {
    TEST_DESCRIPTION("checks a simple conditional transition (condition satisfied)");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF,
                                   this,
                                   &TrafficLightHsm::onOff,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING,
                                   this,
                                   &TrafficLightHsm::onStarting,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);

    registerTransition<TrafficLightHsm>(TrafficLightState::OFF,
                                        TrafficLightState::STARTING,
                                        TrafficLightEvent::TURN_ON,
                                        this,
                                        &TrafficLightHsm::onNextStateTransition,
                                        &TrafficLightHsm::checkConditionOff2On);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "turn on"));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}

TEST_F(TrafficLightHsm, transition_conditional_simple_false) {
    TEST_DESCRIPTION("checks a simple conditional transition (condition not satisfied)");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF,
                                   this,
                                   &TrafficLightHsm::onOff,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING,
                                   this,
                                   &TrafficLightHsm::onStarting,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);

    registerTransition<TrafficLightHsm>(TrafficLightState::OFF,
                                        TrafficLightState::STARTING,
                                        TrafficLightEvent::TURN_ON,
                                        this,
                                        &TrafficLightHsm::onNextStateTransition,
                                        &TrafficLightHsm::checkConditionOff2On);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "ignore"));
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "turn off"));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::OFF);
    EXPECT_EQ(mTransitionCounterNextState, 0);

    EXPECT_EQ(mFailedTransitionCounter, 3);
    EXPECT_EQ(mLastFailedTransition, TrafficLightEvent::TURN_ON);
    ASSERT_EQ(mLastFailedTransitionArgs.size(), 1);
    EXPECT_EQ(mLastFailedTransitionArgs[0].toString(), "turn off");
}

TEST_F(TrafficLightHsm, transition_conditional_multiple) {
    TEST_DESCRIPTION(
        "checks a conditional transition when same event is used for two different transitions from the same state");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF,
                                   this,
                                   &TrafficLightHsm::onOff,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING,
                                   this,
                                   &TrafficLightHsm::onStarting,
                                   &TrafficLightHsm::onEnter,
                                   &TrafficLightHsm::onExit);

    registerTransition<TrafficLightHsm>(TrafficLightState::OFF,
                                        TrafficLightState::STARTING,
                                        TrafficLightEvent::TURN_ON,
                                        this,
                                        &TrafficLightHsm::onNextStateTransition,
                                        &TrafficLightHsm::checkConditionOff2On);
    registerTransition<TrafficLightHsm>(TrafficLightState::OFF,
                                        TrafficLightState::OFF,
                                        TrafficLightEvent::TURN_ON,
                                        this,
                                        &TrafficLightHsm::onNextStateTransition,
                                        &TrafficLightHsm::checkConditionOff2Off);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));              // fail
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "ignore"));    // fail
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "turn off"));   // ok: off->off
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "turn on"));    // ok: off->starting
    ASSERT_FALSE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "turn off"));  // fail

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 2);

    EXPECT_EQ(mFailedTransitionCounter, 3);
    EXPECT_EQ(mLastFailedTransition, TrafficLightEvent::TURN_ON);
    ASSERT_EQ(mLastFailedTransitionArgs.size(), 1);
    EXPECT_EQ(mLastFailedTransitionArgs[0].toString(), "turn off");
}

// NOTE: test is obsolete with introduction of parallel feature
// TEST_F(TrafficLightHsm, transition_conditional_multiple_valid)
// {
//     TEST_DESCRIPTION("if there are multiple valid transitions HSM will pick self-transition or the first applicable one based
//     on registration order");

//     //-------------------------------------------
//     // PRECONDITIONS
//     registerState<TrafficLightHsm>(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter,
//     &TrafficLightHsm::onExit); registerState<TrafficLightHsm>(TrafficLightState::STARTING, this,
//     &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

//     registerTransition<TrafficLightHsm>(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON,
//     this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2On);
//     registerTransition<TrafficLightHsm>(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::TURN_ON, this,
//     &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2Off);

//     //-------------------------------------------
//     // ACTIONS
//     // OFF -> STARTING will be used because it was registered first and doesnt have condition
//     ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION, "any"));

//     //-------------------------------------------
//     // VALIDATION
//     EXPECT_EQ(getLastActiveState(), TrafficLightState::OFF);
//     EXPECT_EQ(mTransitionCounterNextState, 1);
// }

// NOTE: test is obsolete with introduction of parallel feature
// TEST_F(TrafficLightHsm, transition_multiple_valid)
// {
//     TEST_DESCRIPTION("if there are multiple valid transitions HSM will pick the first applicable one based on registration
//     order");

//     //-------------------------------------------
//     // PRECONDITIONS
//     registerState<TrafficLightHsm>(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter,
//     &TrafficLightHsm::onExit); registerState<TrafficLightHsm>(TrafficLightState::STARTING, this,
//     &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

//     registerTransition<TrafficLightHsm>(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::TURN_ON, this,
//     &TrafficLightHsm::onNextStateTransition); registerTransition<TrafficLightHsm>(TrafficLightState::OFF,
//     TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition);

//     //-------------------------------------------
//     // ACTIONS
//     // OFF -> STARTING will be used because it was registered first
//     ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, TIMEOUT_SYNC_TRANSITION));

//     //-------------------------------------------
//     // VALIDATION
//     EXPECT_EQ(getLastActiveState(), TrafficLightState::OFF);
//     EXPECT_EQ(mTransitionCounterNextState, 1);
// }
