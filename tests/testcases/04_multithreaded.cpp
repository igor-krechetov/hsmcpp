// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"
#ifndef WIN32
  #include <signal.h>
#endif

TEST_F(ABCHsm, multithreaded_entrypoint_cancelation) {
    TEST_DESCRIPTION("entrypoint transitions should be atomic and can't be canceled");
    /*
    @startuml
    title multithreaded_entrypoint_cancelation

    state A #orange: onAExit
    state P1 {
        [*] -> B
    }

    A -[#green,bold]> P1: **E1**
    P1 -[#red,bold]> C: **E2**
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, nullptr, nullptr, &ABCHsm::onSyncAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::A, AbcEvent::E1);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    // this should trigger A -> [P1 -> B]
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation(false));  // wait for A::onExit

    // send new event with clearQueue=TRUE
    transitionWithQueueClear(AbcEvent::E2);

    unblockNextStep();               // allow A::onExit to continue
    ASSERT_TRUE(waitAsyncOperation(false));  // wait for B state to activate

    // NOTE: this is the main validation point. In case of an error state would be C since we would never go into B
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));
    unblockNextStep();  // allow B::onStateChanged to continue

    ASSERT_TRUE(waitAsyncOperation()); // wait for C state to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
}

// NOTE: Disable tests because we don't have signals on Windows platfroms
#ifndef WIN32
ABCHsm *gABCHsmInstance = nullptr;

void sigHandler(int signo, siginfo_t *info, void *context) {
    if (nullptr != gABCHsmInstance) {
        gABCHsmInstance->transitionInterruptSafe(AbcEvent::E1);
    }
}

TEST_F(ABCHsm, multithreaded_transition_from_interrupt) {
    TEST_DESCRIPTION("Simple transition from interrupts");

    //-------------------------------------------
    // PRECONDITIONS
    gABCHsmInstance = this;

    registerState(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

    initializeHsm();

    struct sigaction act = {0};

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sigHandler;
    ASSERT_NE(sigaction(SIGUSR1, &act, NULL), (-1));

    //-------------------------------------------
    // ACTIONS
    raise(SIGUSR1);
    ASSERT_TRUE(waitAsyncOperation());

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}
#endif  // !WIN32