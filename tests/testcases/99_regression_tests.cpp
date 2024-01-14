// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, issue_pr_8__invalid_std_timers) {
    TEST_DESCRIPTION("Invalid restart logic for STD timers");
    // NOTE: based on https://github.com/igor-krechetov/hsmcpp/pull/8
    /*
    Let's say I have started a timer on state entry, then stopped it at the same state exit.
    Then, i created internal callback in the same state, reacting to the timer events, and calling a condition callback.
    Essentially what i wanted to have is a periodic callback that is bound to the state itself, that starts being called when
    you enter the state and stops once you leave it. Below is the image from QT creator.

    In the case i have described, everything works as expected. But, if i start another timer in the entry of the same state,
    the callbacks are being called constantly, without regard for the timer interval. It is enough to just start another timer,
    not even use the callback in anything else.
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 1;
    const TimerID_t timer2 = 2;
    const int timerDuration = 100;

    registerTimer(timer1, AbcEvent::E1);
    registerTimer(timer2, AbcEvent::E2);
    registerState(AbcState::A);
    registerStateAction(AbcState::A,
                        hsmcpp::StateActionTrigger::ON_STATE_ENTRY,
                        hsmcpp::StateAction::START_TIMER,
                        timer1,
                        timerDuration,
                        false);
    registerStateAction(AbcState::A,
                        hsmcpp::StateActionTrigger::ON_STATE_ENTRY,
                        hsmcpp::StateAction::START_TIMER,
                        timer2,
                        timerDuration,
                        false);
    registerSelfTransition<ABCHsm>(AbcState::A,
                                   AbcEvent::E1,
                                   hsmcpp::TransitionType::INTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onE1Transition);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timerDuration * 2));

    //-------------------------------------------
    // VALIDATION
    EXPECT_GT(mTransitionCounterE1, 0);
    EXPECT_LE(mTransitionCounterE1, 2);
}
