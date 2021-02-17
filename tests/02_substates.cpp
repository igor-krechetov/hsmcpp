#include "hsm/TrafficLightHsm.hpp"
#include "hsm/ABCHsm.hpp"

TEST_F(TrafficLightHsm, substate_enter)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    //-------------------------------------------
    // ACTIONS

    //-------------------------------------------
    // VALIDATION
    GTEST_SKIP() << "NOT IMPLEMENTED";
}

TEST_F(TrafficLightHsm, substate_exit_single)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true));
    ASSERT_EQ(getCurrentState(), TrafficLightState::STARTING);
    ASSERT_TRUE(transitionEx(TrafficLightEvent::NEXT_STATE, false, true));
    ASSERT_EQ(getCurrentState(), TrafficLightState::RED);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_OFF, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterStarting, 1);
    EXPECT_EQ(mStateCounterRed, 1);
    EXPECT_EQ(mStateCounterOff, 1);
    EXPECT_EQ(mStateCounterTurnOff, 1);
}

TEST_F(TrafficLightHsm, substate_exit_multiple_layers)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    setupDefault();

    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_ON, false, true));
    ASSERT_EQ(getCurrentState(), TrafficLightState::STARTING);
    ASSERT_TRUE(transitionEx(TrafficLightEvent::NEXT_STATE, false, true));
    ASSERT_EQ(getCurrentState(), TrafficLightState::RED);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(TrafficLightEvent::TURN_OFF, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), TrafficLightState::OFF);
    EXPECT_EQ(mStateCounterStarting, 1);
    EXPECT_EQ(mStateCounterRed, 1);
    EXPECT_EQ(mStateCounterOff, 1);
    EXPECT_EQ(mStateCounterTurnOff, 1);

    // TODO: implement multiple depth check

    GTEST_SKIP() << "NOT IMPLEMENTED";
}

TEST_F(ABCHsm, substate_safe_registration)
{
    TEST_DESCRIPTION("If HSM is compiled with safety check then it should prevent cyclic and multiple inclusions of substates. "
                     "This test will fail if HSM_ENABLE_SAFE_STRUCTURE is not defined");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(AbcState::A, this, &ABCHsm::onA, nullptr, nullptr);
    registerState(AbcState::B, this, &ABCHsm::onB, nullptr, nullptr);
    registerState(AbcState::C, this, &ABCHsm::onC, nullptr, nullptr);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::A) );

    //-------------------------------------------
    // ACTIONS

    // each state can be a part of only one substate
    EXPECT_FALSE( registerSubstate(AbcState::P2, AbcState::A) );// A is already part of P1

    // prevent recursion
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::B) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::P2) );
    EXPECT_FALSE( registerSubstate(AbcState::P2, AbcState::P1) );// P1 -> B2 -X P1

    EXPECT_TRUE( registerSubstate(AbcState::P4, AbcState::C) );
    EXPECT_TRUE( registerSubstate(AbcState::P4, AbcState::P3) );
    EXPECT_FALSE( registerSubstate(AbcState::P3, AbcState::P4) );// P3 -X P4 -> P3

    EXPECT_TRUE( registerSubstate(AbcState::P3, AbcState::P1) );
    EXPECT_FALSE( registerSubstate(AbcState::P2, AbcState::P4) );// prevent recursion: (P4) -> P3 -> P1 -> P2 -X (P4)

    //-------------------------------------------
    // VALIDATION
}