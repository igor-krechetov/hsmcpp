// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/TrafficLightHsm.hpp"

TEST_F(TrafficLightHsm, initial_state)
{
    TEST_DESCRIPTION("after creation FSM should be in it's initial state");
    
    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    TrafficLightState curState = getLastActiveState();

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(curState, TrafficLightState::OFF);
}

TEST_F(TrafficLightHsm, register_states)
{
    TEST_DESCRIPTION("simple test to check registration of two states and transition between them");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF, this, &TrafficLightHsm::onOff);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::STARTING}));
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 1);
}

TEST_F(TrafficLightHsm, register_state_without_action)
{
    TEST_DESCRIPTION("states without actions should be allowed");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    registerState(TrafficLightState::OFF);
    registerState(TrafficLightState::STARTING);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, HSM_WAIT_INDEFINITELY));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::STARTING}));
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, register_same_state_twice)
{
    TEST_DESCRIPTION("registering same state twice should overwrite previous actions");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // ACTIONS
    registerState<TrafficLightHsm>(TrafficLightState::STARTING, this, &TrafficLightHsm::onDummy);

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, HSM_WAIT_INDEFINITELY));
    EXPECT_EQ(mStateCounterDummy, 1);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, err_register_without_handler)
{
    TEST_DESCRIPTION("registering an action without a handler should be ignored");
    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // ACTIONS

    registerState<TrafficLightHsm>(TrafficLightState::STARTING, nullptr, &TrafficLightHsm::onStarting);

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, HSM_WAIT_INDEFINITELY));
    EXPECT_EQ(mStateCounterDummy, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, state_args_test)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    registerState<TrafficLightHsm>(TrafficLightState::OFF, this, &TrafficLightHsm::onOff);
    registerState<TrafficLightHsm>(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON);

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(transitionSync(TrafficLightEvent::TURN_ON, HSM_WAIT_INDEFINITELY, 12, "string", 12.75, false));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {TrafficLightState::STARTING}));
    EXPECT_EQ(mStateCounterStarting, 1);

    EXPECT_EQ(mArgsStarting.size(), 4);

    EXPECT_TRUE(mArgsStarting[0].isNumeric());
    EXPECT_EQ(mArgsStarting[0].toInt64(), 12);

    EXPECT_TRUE(mArgsStarting[1].isString());
    EXPECT_STREQ(mArgsStarting[1].toString().c_str(), "string");

    EXPECT_TRUE(mArgsStarting[2].isNumeric());
    EXPECT_FLOAT_EQ(mArgsStarting[2].toDouble(), 12.75);

    EXPECT_TRUE(mArgsStarting[3].isBool());
    EXPECT_FALSE(mArgsStarting[3].toBool());
}