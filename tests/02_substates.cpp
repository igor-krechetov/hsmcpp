#include "hsm/TrafficLightHsm.hpp"
#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, substate_entrypoint)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(AbcState::A, this, &ABCHsm::onA, nullptr, nullptr);
    registerState(AbcState::B, this, &ABCHsm::onB, nullptr, nullptr);
    registerState(AbcState::C, this, &ABCHsm::onC, nullptr, nullptr);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::B, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::C) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(AbcEvent::E1, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), AbcState::B);
}


TEST_F(ABCHsm, substate_entrypoint_substate)
{
    TEST_DESCRIPTION("entry point of a substate could be another state with substates");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(AbcState::A, this, &ABCHsm::onA, nullptr, nullptr);
    registerState(AbcState::B, this, &ABCHsm::onB, nullptr, nullptr);
    registerState(AbcState::C, this, &ABCHsm::onC, nullptr, nullptr);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::P2, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P2, AbcState::P3, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P3, AbcState::B, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P3, AbcState::C) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(AbcEvent::E1, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), AbcState::B);
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
    EXPECT_EQ(mTransitionCounterTurnOff, 1);
}

TEST_F(ABCHsm, substate_exit_multiple_layers)
{
    TEST_DESCRIPTION("Validate that exiting from multiple depth states on a top level transition is correctly handled");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(AbcState::A, this, &ABCHsm::onA, nullptr, nullptr);
    registerState(AbcState::B, this, &ABCHsm::onB, nullptr, nullptr);
    registerState(AbcState::C, this, &ABCHsm::onC, nullptr, nullptr);
    registerState(AbcState::D, this, &ABCHsm::onD, nullptr, nullptr);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::B, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::P2) );
    EXPECT_TRUE( registerSubstate(AbcState::P2, AbcState::C, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P2, AbcState::P3) );
    EXPECT_TRUE( registerSubstate(AbcState::P3, AbcState::D, true) );

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1, nullptr, nullptr);
    registerTransition(AbcState::B, AbcState::P2, AbcEvent::E1, nullptr, nullptr);
    registerTransition(AbcState::C, AbcState::P3, AbcEvent::E1, nullptr, nullptr);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E2, nullptr, nullptr);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionEx(AbcEvent::E1, false, true));
    ASSERT_EQ(getCurrentState(), AbcState::B);

    ASSERT_TRUE(transitionEx(AbcEvent::E1, false, true));
    ASSERT_EQ(getCurrentState(), AbcState::C);

    ASSERT_TRUE(transitionEx(AbcEvent::E1, false, true));
    ASSERT_EQ(getCurrentState(), AbcState::D);

    ASSERT_TRUE(transitionEx(AbcEvent::E2, false, true));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), AbcState::A);
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
    registerState(AbcState::A, this, &ABCHsm::onA, nullptr, nullptr);
    registerState(AbcState::B, this, &ABCHsm::onB, nullptr, nullptr);
    registerState(AbcState::C, this, &ABCHsm::onC, nullptr, nullptr);

    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::A, true) );

    //-------------------------------------------
    // ACTIONS

    // each state can be a part of only one substate
    EXPECT_FALSE( registerSubstate(AbcState::P2, AbcState::A, true) );// A is already part of P1

    // prevent recursion
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::B) );
    EXPECT_TRUE( registerSubstate(AbcState::P1, AbcState::P2) );
    EXPECT_FALSE( registerSubstate(AbcState::P2, AbcState::P1, true) );// P1 -> B2 -X P1

    EXPECT_TRUE( registerSubstate(AbcState::P4, AbcState::C, true) );
    EXPECT_TRUE( registerSubstate(AbcState::P4, AbcState::P3) );
    EXPECT_FALSE( registerSubstate(AbcState::P3, AbcState::P4, true) );// P3 -X P4 -> P3

    EXPECT_TRUE( registerSubstate(AbcState::P3, AbcState::P1, true) );
    EXPECT_FALSE( registerSubstate(AbcState::P2, AbcState::P4, true) );// prevent recursion: (P4) -> P3 -> P1 -> P2 -X (P4)

    //-------------------------------------------
    // VALIDATION
}