// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/AsyncHsm.hpp"
#include "hsm/ABCHsm.hpp"
#ifndef WIN32
  #include <signal.h>
#endif

TEST_F(AsyncHsm, multithreaded_entrypoint_cancelation)
{
    TEST_DESCRIPTION("entrypoint transitions should be atomic and can't be canceled");
    /*
    @startuml
    left to right direction
    title multithreaded_entrypoint_cancelation

    state A #orange: onExit

    A -[#green,bold]-> P1: NEXT_STATE
    P1 -[#red,bold]-> C: EXIT_SUBSTATE
    C --> A: NEXT_STATE
    state P1 {
        [*] --> B
    }
    P1 --> A : E3
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<AsyncHsm>(AsyncHsmState::A, this, nullptr, nullptr, &AsyncHsm::onExit);
    registerState<AsyncHsm>(AsyncHsmState::B, this, &AsyncHsm::onStateChanged);
    registerState<AsyncHsm>(AsyncHsmState::C, this, &AsyncHsm::onStateChanged);

    ASSERT_TRUE(registerSubstateEntryPoint(AsyncHsmState::P1, AsyncHsmState::B));

    registerTransition(AsyncHsmState::A, AsyncHsmState::P1, AsyncHsmEvent::NEXT_STATE);
    registerTransition(AsyncHsmState::P1, AsyncHsmState::C, AsyncHsmEvent::EXIT_SUBSTATE);
    registerTransition(AsyncHsmState::C, AsyncHsmState::A, AsyncHsmEvent::NEXT_STATE);

    initializeHsm();

    ASSERT_EQ(getLastActiveState(), AsyncHsmState::A);

    //-------------------------------------------
    // ACTIONS
    // this should trigger A -> [P1 -> B]
    transition(AsyncHsmEvent::NEXT_STATE);
    waitAsyncOperation(200);// wait for A::onExit

    // send new event with clearQueue=TRUE
    transitionWithQueueClear(AsyncHsmEvent::EXIT_SUBSTATE);

    unblockNextStep();// allow A::onExit to continue
    waitAsyncOperation(200);// wait for B::onStateChanged

    // NOTE: this is the main validation point. In case of an error state would be C since we would never go into B
    ASSERT_EQ(getLastActiveState(), AsyncHsmState::B);
    unblockNextStep();// allow B::onStateChanged to continue

    waitAsyncOperation(200);// wait for C::onStateChanged
    ASSERT_EQ(getLastActiveState(), AsyncHsmState::C);
    unblockNextStep();// allow C::onStateChanged to continue

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AsyncHsmState::C);
}

// NOTE: Qt doesn't support cancelation of already posted events. So deleting dispatcher before
//       all events are processed will result in a crash.
#ifndef TEST_HSM_QT
TEST_F(ABCHsm, multithreaded_deleting_running_dispatcher)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    for (int i = 0; i < 100; ++i)
    {
        transition(AbcEvent::E1);
    }

    //-------------------------------------------
    // VALIDATION
}
#endif // !TEST_HSM_QT

#ifndef WIN32
// Disable test because we don't have signals on Windows platfroms

AsyncHsm *gAsyncHsmInstance = nullptr;

void sigHandler(int signo, siginfo_t *info, void *context)
{
    if (nullptr != gAsyncHsmInstance)
    {
        gAsyncHsmInstance->transitionInterruptSafe(AsyncHsmEvent::NEXT_STATE);
    }
}

TEST_F(AsyncHsm, multithreaded_transition_from_interrupt)
{
    TEST_DESCRIPTION("Simple transition from interrupts");

    //-------------------------------------------
    // PRECONDITIONS
    gAsyncHsmInstance = this;

    registerState(AsyncHsmState::A);
    registerState<AsyncHsm>(AsyncHsmState::B, this, &AsyncHsm::onStateChanged);

    registerTransition(AsyncHsmState::A, AsyncHsmState::B, AsyncHsmEvent::NEXT_STATE);
    registerTransition(AsyncHsmState::B, AsyncHsmState::A, AsyncHsmEvent::NEXT_STATE);

    initializeHsm();

    struct sigaction act = { 0 };

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sigHandler;
    ASSERT_NE(sigaction(SIGUSR1, &act, NULL), (-1));

    //-------------------------------------------
    // ACTIONS
    raise(SIGUSR1);
    ASSERT_TRUE(waitAsyncOperation(1000));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getLastActiveState(), AsyncHsmState::B);
}
#endif // !WIN32