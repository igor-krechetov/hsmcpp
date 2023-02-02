// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, timers_onentry) {
    TEST_DESCRIPTION("Validate that timer actions can be executed on state entry");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::C);
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_onexit) {
    TEST_DESCRIPTION("Validate that timer actions can be executed on state exit");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::A,
                        ABCHsm::StateActionTrigger::ON_STATE_EXIT,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::C);
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_multiple_actions) {
    TEST_DESCRIPTION("Validate that timer actions can be executed on state entry");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const TimerID_t timer2 = 2;
    const int timer1Duration = 100;
    const int timer2Duration = 200;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerState<ABCHsm>(AbcState::D);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E3);

    registerTimer(timer1, AbcEvent::E2);
    registerTimer(timer2, AbcEvent::E3);

    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer2,
                        timer2Duration,
                        true);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    std::this_thread::sleep_for(std::chrono::milliseconds(timer2Duration - timer1Duration));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::D);
}

TEST_F(ABCHsm, timers_singleshot) {
    TEST_DESCRIPTION("Validate support for single shot timer");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::C);
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_repeating) {
    TEST_DESCRIPTION("Validate support for repeating timers");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration * 2 + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::D);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterD, 1);
}

TEST_F(ABCHsm, timers_stop) {
    TEST_DESCRIPTION("Validate stop timer action");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    registerStateAction(AbcState::C, ABCHsm::StateActionTrigger::ON_STATE_ENTRY, ABCHsm::StateAction::STOP_TIMER, timer1);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration * 3));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::C);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterD, 0);
}

TEST_F(ABCHsm, timers_restart) {
    TEST_DESCRIPTION("Validate restart timer action");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const TimerID_t timer2 = 2;
    const int timer1Duration = 600;
    const int timer2Duration = 200;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::B, AbcEvent::E3);

    registerTimer(timer1, AbcEvent::E3);
    registerTimer(timer2, AbcEvent::E2);

    registerStateAction(AbcState::A,
                        ABCHsm::StateActionTrigger::ON_STATE_EXIT,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    registerStateAction(AbcState::B,
                        ABCHsm::StateActionTrigger::ON_STATE_ENTRY,
                        ABCHsm::StateAction::START_TIMER,
                        timer2,
                        timer2Duration,
                        true);
    registerStateAction(AbcState::C, ABCHsm::StateActionTrigger::ON_STATE_ENTRY, ABCHsm::StateAction::RESTART_TIMER, timer1);

    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer2Duration + 50));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    // elapsed 250ms. at this point timer1 should have 350ms left if it hasn't been restarted. let's wait for 400ms to check
    // that
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration - timer2Duration));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    // wait for remaining time
    std::this_thread::sleep_for(std::chrono::milliseconds(timer2Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::B);
}

TEST_F(ABCHsm, timers_delete_running) {
    TEST_DESCRIPTION("Delete HSM instance while timer is still running");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::B, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::A,
                        ABCHsm::StateActionTrigger::ON_STATE_EXIT,
                        ABCHsm::StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::B);

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    //-------------------------------------------
    // VALIDATION
    // NOTE: in case of an error test will crash. using "death tests" is impossible due to multiple threads
}

TEST_F(ABCHsm, timers_start_from_code) {
    TEST_DESCRIPTION("Validate support for starting timers from code");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::B);
    EXPECT_EQ(mStateCounterB, 1);
}

TEST_F(ABCHsm, timers_stop_from_code) {
    TEST_DESCRIPTION("Validate support for stopping timers from code");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));
    stopTimer(timer1);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::A);
    EXPECT_EQ(mStateCounterB, 0);
}

TEST_F(ABCHsm, timers_restart_from_code) {
    TEST_DESCRIPTION("Validate support for stopping timers from code");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));
    restartTimer(timer1);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2 + 10));
    ASSERT_EQ(getLastActiveState(), AbcState::A);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AbcState::B);
    EXPECT_EQ(mStateCounterB, 1);
}