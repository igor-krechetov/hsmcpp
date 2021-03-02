#include "hsm/TrafficLightHsm.hpp"

TEST_F(TrafficLightHsm, simple_transition)
{
    TEST_DESCRIPTION("Simple transition between two states");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::STARTING);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 1);
}

TEST_F(TrafficLightHsm, transition_with_args)
{
    TEST_DESCRIPTION("Test if args are correctly passed to transition action handler");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, nullptr, nullptr, nullptr, nullptr);
    registerState(TrafficLightState::STARTING, nullptr, nullptr, nullptr, nullptr);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true, 12, "string", 12.75, false));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 1);

    EXPECT_EQ(mTransitionArgsNextState.size(), 4);

    EXPECT_TRUE(mTransitionArgsNextState[0].isNumeric());
    EXPECT_EQ(mTransitionArgsNextState[0].toInt64(), 12);

    EXPECT_TRUE(mTransitionArgsNextState[1].isString());
    EXPECT_STREQ(mTransitionArgsNextState[1].toString().c_str(), "string");

    EXPECT_TRUE(mTransitionArgsNextState[2].isNumeric());
    EXPECT_FLOAT_EQ(mTransitionArgsNextState[2].toDouble(), 12.75);

    EXPECT_TRUE(mTransitionArgsNextState[3].isBool());
    EXPECT_FALSE(mTransitionArgsNextState[3].toBool());
}

TEST_F(TrafficLightHsm, transition_non_existent)
{
    TEST_DESCRIPTION("Check that non registered transitions don't change HSM state");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    TrafficLightState prevState = getCurrentState();

    ASSERT_FALSE(transitionEx(TrafficLightEvent::NEXT_STATE, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), prevState);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, transition_cancel_on_exit)
{
    TEST_DESCRIPTION("It should be possible to cancel transition if OnExit handler returns FALSE");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, nullptr, &TrafficLightHsm::onExitCancel);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, nullptr);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterExitCancel, 1);
    EXPECT_EQ(mStateCounterExit, 0);
    EXPECT_EQ(mStateCounterEnter, 0);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, transition_cancel_on_enter)
{
    TEST_DESCRIPTION("It should be possible to cancel transition if onEnter handler returns FALSE");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnterCancel, nullptr);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true));

    //-------------------------------------------
    // VALIDATION

    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterExit, 1);
    EXPECT_EQ(mStateCounterEnter, 1);
    EXPECT_EQ(mStateCounterEnterCancel, 1);
    EXPECT_EQ(mStateCounterOff, 1);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, transition_self)
{
    TEST_DESCRIPTION("Self transition should not trigger any state handlers. Only transition handler must be excecuted");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::NEXT_STATE, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterExit, 0);
    EXPECT_EQ(mStateCounterEnter, 0);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}

TEST_F(TrafficLightHsm, transition_entrypoint_raicecondition)
{
    TEST_DESCRIPTION("entrypoint transitions should be atomic and can't be cancled");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS
    TrafficLightState prevState = getCurrentState();

    ASSERT_FALSE(transitionEx(TrafficLightEvent::NEXT_STATE, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), prevState);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
}

TEST_F(TrafficLightHsm, transition_conditional_simple_true)
{
    TEST_DESCRIPTION("checks a simple conditional transition (condition satisfied)");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2On);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "turn on"));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}

TEST_F(TrafficLightHsm, transition_conditional_simple_false)
{
    TEST_DESCRIPTION("checks a simple conditional transition (condition not satisfied)");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2On);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true));
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "ignore"));
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "turn off"));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mTransitionCounterNextState, 0);
}

TEST_F(TrafficLightHsm, transition_conditional_multiple)
{
    TEST_DESCRIPTION("checks a conditional transition when same event is used for two different transitions from the same state");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2On);
    registerTransition(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2Off);

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true));
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "ignore"));
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "turn off"));
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "turn on"));
    ASSERT_FALSE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "turn off"));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 2);
}

TEST_F(TrafficLightHsm, transition_conditional_multiple_valid)
{
    TEST_DESCRIPTION("if there are multiple valid transitions HSM will pick the first applicable one based on registration order");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2On);
    registerTransition(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, &TrafficLightHsm::checkConditionOff2Off);

    //-------------------------------------------
    // ACTIONS
    // OFF -> STARTING will be used because it was registered first and doesnt have condition
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true, "any"));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::STARTING);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}

TEST_F(TrafficLightHsm, transition_multiple_valid)
{
    TEST_DESCRIPTION("if there are multiple valid transitions HSM will pick the first applicable one based on registration order");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, &TrafficLightHsm::onEnter, &TrafficLightHsm::onExit);

    registerTransition(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, nullptr);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onNextStateTransition, nullptr);

    //-------------------------------------------
    // ACTIONS
    // OFF -> STARTING will be used because it was registered first
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mTransitionCounterNextState, 1);
}
