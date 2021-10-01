#include "hsm/ABCHsm.hpp"
#include <chrono>
#include <thread>

TEST_F(ABCHsm, state_actions_simple)
{
    TEST_DESCRIPTION("Validate that state actions can be executed on state entry and exit");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::A, AbcEvent::E3);

    registerStateAction(AbcState::B, ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::TRANSITION, static_cast<int>(AbcEvent::E2));
    registerStateAction(AbcState::B, ABCHsm::StateActionTrigger::ON_STATE_EXIT,
                        ABCHsm::StateAction::TRANSITION, static_cast<int>(AbcEvent::E3));
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // wait a bit for second transition to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::A);
}

TEST_F(ABCHsm, state_actions_multiple)
{
    TEST_DESCRIPTION("Validate that multiple state actions will be executed on state entry");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::A, AbcEvent::E3);

    registerStateAction(AbcState::B, ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::TRANSITION, static_cast<int>(AbcEvent::E2));
    registerStateAction(AbcState::B, ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::TRANSITION, static_cast<int>(AbcEvent::E3));
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // wait a bit for second transition to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::A);
}

TEST_F(ABCHsm, state_actions_args)
{
    TEST_DESCRIPTION("transition actions should support arguments");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerStateAction(AbcState::B, ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::TRANSITION, static_cast<int>(AbcEvent::E2), 123, "string arg");
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // wait a bit for second transition to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::C);
    ASSERT_EQ(mArgsC.size(), 2);
    ASSERT_TRUE(mArgsC[0].isNumeric());
    ASSERT_TRUE(mArgsC[1].isString());
    ASSERT_EQ(mArgsC[0].toInt64(), 123);
    ASSERT_EQ(mArgsC[1].toString(), "string arg");
}
