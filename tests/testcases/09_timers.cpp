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
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_onexit) {
    TEST_DESCRIPTION("Validate that timer actions can be executed on state exit");
    /*
    @startuml
    title timers_onexit

    state A #orange: onExit: start_timer(E2)

    A -[#green,bold]> B: E1
    B -[#green,bold]> C #LightGreen: E2
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 2;
    const int timer1Duration = 500;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::A,
                        StateActionTrigger::ON_STATE_EXIT,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    initializeHsm();

    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation(false));// wait for B to activate and block HSM
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    unblockNextStep();// allow HSM to continue

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_multiple_actions) {
    TEST_DESCRIPTION("Validate that timer actions can be executed on state entry");
    /*
    @startuml
    title timers_multiple_actions

    state A #orange
    state B: onEntry: start_timer(E2, 100, true)
    state B: onEntry: start_timer(E3, 300, true)

    A -[#green,bold]> B: E1
    B -[#green,bold]> C: E2
    C -[#green,bold]> D #LightGreen: E3
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 3;
    const TimerID_t timer2 = 4;
    const int timer1Duration = 100;
    const int timer2Duration = 200;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E3);

    registerTimer(timer1, AbcEvent::E2);
    registerTimer(timer2, AbcEvent::E3);

    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer2,
                        timer2Duration,
                        true);
    initializeHsm();

    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation(false));// wait for B to activate and block HSM
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    unblockNextStep();// allow HSM to continue

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration));
    ASSERT_TRUE(waitAsyncOperation(50, false));// wait for C to activate and block HSM
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    unblockNextStep();// allow HSM to continue

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    std::this_thread::sleep_for(std::chrono::milliseconds(timer2Duration - timer1Duration));
    ASSERT_TRUE(waitAsyncOperation(50, true));// wait for D to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));
}

TEST_F(ABCHsm, timers_singleshot) {
    TEST_DESCRIPTION("Validate support for single shot timer");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 5;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_repeating) {
    TEST_DESCRIPTION("Validate support for repeating timers");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 6;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerSelfTransition<ABCHsm>(AbcState::C, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    // timer will expire 2-3 times depending on thread scheduling
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration * 2 + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_GE(mStateCounterC, 2);
    EXPECT_LE(mStateCounterC, 3);
}

TEST_F(ABCHsm, timers_repeating_delay) {
    TEST_DESCRIPTION("Repeating timer event should be fired after equal intervals");

    /*
    @startuml
    scale 50 as 70 pixels
    title timers_repeating_delay
    concise "Timer" as Timer
    concise "Task" as Task

    @0
    Timer is Running
    Task is {hidden}
    @100
    Timer is Running
    Timer -> Task@100: trigger
    @200
    Timer is Running
    Timer -> Task@200: trigger
    @300
    Timer is {hidden}
    Timer -> Task@300: trigger


    @100
    Task is Working
    @180
    Task is {hidden}
    @200
    Task is Working
    @270
    Task is {hidden}
    @300
    Task is Working
    @380
    Task is {hidden}

    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 7;
    const int timer1Duration = 100;
    const int timer1Repeat = 2;
    std::chrono::time_point<std::chrono::steady_clock> timerStartTime;
    int correctIterations = 0;
    int iterationsCount = 0;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerSelfTransition(AbcState::B, AbcEvent::E2, hsmcpp::TransitionType::INTERNAL_TRANSITION,
        [&](const VariantVector_t& args){
            const auto eventTriggerTime = std::chrono::steady_clock::now();
            const int iterationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(eventTriggerTime - timerStartTime).count();

            // NOTE: this is not precise since timer event is dispatched on a different thread.
            //       so need to account for iterationDuration being reduced by 1-2ms
            timerStartTime = std::chrono::steady_clock::now();

            // This measurement is not precise so need to count for operational costs
            if ((iterationDuration >= (timer1Duration - 5)) && (iterationDuration <= (timer1Duration + 5))) {
                ++correctIterations;
            }

            ++iterationsCount;
            std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));
        });

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    timerStartTime = std::chrono::steady_clock::now();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    // timer will expire 2-3 times depending on thread scheduling
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration * timer1Repeat + static_cast<int>(timer1Duration * 0.5)));

    //-------------------------------------------
    // VALIDATION
    EXPECT_GE(iterationsCount, timer1Repeat);
    EXPECT_EQ(correctIterations, iterationsCount);
}

TEST_F(ABCHsm, timers_stop) {
    TEST_DESCRIPTION("Validate stop timer action");

    /*
    TODO
    @startuml
    title timers_stop

    state A
    state B #Orange: onEntry: start_timer(E2)
    state C #LightGreen: onEntry: stop_timer(E2)\nonC
    state D: onD

    [*] -> A
    A -> B: E1
    B -> C: E2
    C -> D: E2

    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 8;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    registerStateAction(AbcState::C, StateActionTrigger::ON_STATE_ENTRY, StateAction::STOP_TIMER, timer1);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(waitAsyncOperation());// wait for C to activate
    // std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration * 3));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterD, 0);
}

TEST_F(ABCHsm, timers_restart) {
    TEST_DESCRIPTION("Validate restart timer action");
    /*
    @startuml
    left to right direction
    title substate_entrypoints_multiple_behavioral

    state A: **onStateExit: startTimer(E3, single, 600ms)**
    state B #orange: **onStateEntry: startTimer(E2, single, 200ms)**
    state C: onStateEntry: restartTimer(E3)

    A --> B: E1
    B -up[#green,bold]-> C: E2
    C --> B: E3
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 9;
    const TimerID_t timer2 = 10;
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
                        StateActionTrigger::ON_STATE_EXIT,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        true);
    registerStateAction(AbcState::B,
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        timer2,
                        timer2Duration,
                        true);
    registerStateAction(AbcState::C, StateActionTrigger::ON_STATE_ENTRY, StateAction::RESTART_TIMER, timer1);

    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer2Duration + 50));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    // elapsed 250ms. at this point timer1 should have 350ms left if it hasn't been restarted. let's wait for 400ms to check
    // that
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration - timer2Duration));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));

    // wait for remaining time
    std::this_thread::sleep_for(std::chrono::milliseconds(timer2Duration + 50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, timers_delete_running) {
    TEST_DESCRIPTION("Delete HSM instance while timer is still running");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 11;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::B, AbcEvent::E2);

    registerTimer(timer1, AbcEvent::E2);
    registerStateAction(AbcState::A,
                        StateActionTrigger::ON_STATE_EXIT,
                        StateAction::START_TIMER,
                        timer1,
                        timer1Duration,
                        false);
    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 20));

    //-------------------------------------------
    // VALIDATION
    // NOTE: in case of an error test will crash. using "death tests" is impossible due to multiple threads
}

TEST_F(ABCHsm, timers_start_from_code) {
    TEST_DESCRIPTION("Validate support for starting timers from code");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 12;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration + 50));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    EXPECT_EQ(mStateCounterB, 1);
}

TEST_F(ABCHsm, timers_start_higher_priority) {
    TEST_DESCRIPTION("If during a running timer a new timer with a shorter elapse period is started HSM should "
                     "correctly schedule it");

    /*
    @startuml
    scale 10 as 40 pixels
    title timers_start_higher_priority
    binary "Timer Long" as Timer1
    binary "Timer Short" as Timer2
    concise "HSM" as HSM

    @0
    Timer1 is high
    @100
    Timer1 is low
    Timer1 -> HSM : ignore

    @20
    Timer2 is high
    @40
    Timer2 is low
    Timer2 -> HSM : trigger

    @40
    HSM is E2
    @80
    HSM is {hidden}

    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timerShort = 13;
    const int timerShortDuration = 20;
    const TimerID_t timerLong = 14;
    const int timerLongDuration = timerShortDuration * 5;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::A, AbcState::C, AbcEvent::E2);

    registerTimer(timerLong, AbcEvent::E1);
    registerTimer(timerShort, AbcEvent::E2);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    startTimer(timerLong, timerLongDuration, true);
    // wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(timerShortDuration));
    startTimer(timerShort, timerShortDuration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timerLongDuration + 50));

    //-------------------------------------------
    // VALIDATION
    // Short timer should fire first and Long timer transition must be ignored
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_EQ(mStateCounterB, 0);
    EXPECT_EQ(mStateCounterC, 1);
}

TEST_F(ABCHsm, timers_stop_from_code) {
    TEST_DESCRIPTION("Validate support for stopping timers from code");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 15;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));
    stopTimer(timer1);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    EXPECT_EQ(mStateCounterB, 0);
}

TEST_F(ABCHsm, timers_restart_from_code) {
    TEST_DESCRIPTION("Validate support for stopping timers from code");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 16;
    const int timer1Duration = 100;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));
    restartTimer(timer1);
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2 + 10));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration / 2));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    EXPECT_EQ(mStateCounterB, 1);
}

TEST_F(ABCHsm, timers_is_running) {
    TEST_DESCRIPTION("HSM should provide a way to check if timer is running");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 17;
    const int timer1Duration2 = 2000;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_FALSE(isTimerRunning(timer1));

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration2, true);
    ASSERT_TRUE(isTimerRunning(timer1));

    stopTimer(timer1);
    ASSERT_FALSE(isTimerRunning(timer1));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

TEST_F(ABCHsm, timers_singleshot_is_running) {
    TEST_DESCRIPTION("single shot timers should become inactive after they expire");

    //-------------------------------------------
    // PRECONDITIONS
    const TimerID_t timer1 = 18;
    const int timer1Duration1 = 50;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    registerTimer(timer1, AbcEvent::E1);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_FALSE(isTimerRunning(timer1));

    //-------------------------------------------
    // ACTIONS
    startTimer(timer1, timer1Duration1, true);
    ASSERT_TRUE(isTimerRunning(timer1));
    std::this_thread::sleep_for(std::chrono::milliseconds(timer1Duration1 * 2));
    ASSERT_FALSE(isTimerRunning(timer1));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

// TODO: start already running timer