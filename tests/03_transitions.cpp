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
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, this, &TrafficLightHsm::onArgsTest);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true, 12, "string", 12.75, false));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::STARTING);
    EXPECT_EQ(mStateCounterArgsTest, 1);

    EXPECT_EQ(mArgsTest.size(), 4);

    EXPECT_TRUE(mArgsTest[0].isNumeric());
    EXPECT_EQ(mArgsTest[0].toInt64(), 12);

    EXPECT_TRUE(mArgsTest[1].isString());
    EXPECT_STREQ(mArgsTest[1].toString().c_str(), "string");

    EXPECT_TRUE(mArgsTest[2].isNumeric());
    EXPECT_FLOAT_EQ(mArgsTest[2].toDouble(), 12.75);

    EXPECT_TRUE(mArgsTest[3].isBool());
    EXPECT_FALSE(mArgsTest[3].toBool());
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
    EXPECT_EQ(mStateCounterOnExit, 1);
    EXPECT_EQ(mStateCounterOnEnter, 0);
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
    EXPECT_EQ(mStateCounterOnExit, 1);
    EXPECT_EQ(mStateCounterOnEnter, 2);
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

    registerTransition(TrafficLightState::OFF, TrafficLightState::OFF, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onArgsTest);
    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::NEXT_STATE, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterOnExit, 0);
    EXPECT_EQ(mStateCounterOnEnter, 0);
    EXPECT_EQ(mStateCounterOff, 0);
    EXPECT_EQ(mStateCounterStarting, 0);
    EXPECT_EQ(mStateCounterArgsTest, 1);
}
