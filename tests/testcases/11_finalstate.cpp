// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, finalstate_simple_exitpoint) {
    TEST_DESCRIPTION("HSM should automatically generate event when entering final state");
    /*
    @startuml
    title finalstate_simple_exitpoint

    [*] --> P1
    P1 -[#green,bold]> B #LightGreen: **EXIT1**

    state P1 {
        state A #orange
        state F1 <<end>>
        note right of F1
            EXIT1 event
        end note

        [*] -> A
        A -[#green,bold]> F1: **E1**
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::P1);

    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::EXIT1);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for state A to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for state B to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, finalstate_forward_event) {
    TEST_DESCRIPTION("HSM will trigger same event as exit event if final state didn't have any event registered");
    /*
    @startuml
    title finalstate_forward_event

    [*] --> P1
    P1 -[#green,bold]> B #LightGreen: **E1**

    state P1 {
        state A #orange
        state F1 <<end>>

        [*] -> A
        A -[#green,bold]> F1: **E1**
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::P1);

    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerFinalState(AbcState::F1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for state A to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for state C to activate (exit transition is async)

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

class ParamFixtureFinalState1
    : public ABCHsm,
      public ::testing::WithParamInterface<std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t>> {};

INSTANTIATE_TEST_CASE_P(finalstate,
                        ParamFixtureFinalState1,
                        ::testing::Values(std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E1}), AbcState::C),
                                          std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E3, AbcEvent::E2}),
                                                          AbcState::D)));

TEST_P(ParamFixtureFinalState1, finalstate_multiple_final) {
    TEST_DESCRIPTION("HSM should support multiple path to a final state");
    /*
    @startuml
    title finalstate_multiple_final

    [*] --> P1
    P1 --> C: E1
    P1 --> D: E2

    state P1 {
        state A #orange
        state B
        state F1 <<end>>

        [*] --> A
        A -> B: E3
        A --> F1: E1
        B --> F1: E2
    }
    @enduml
    */

    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);
    registerFinalState(AbcState::F1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E3);
    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::F1, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::C, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::E2);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for state A to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    for (const auto curEvent : argEvents) {
        transition(curEvent);
        ASSERT_TRUE(waitAsyncOperation());
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {expectedState}));
}

TEST_P(ParamFixtureFinalState1, finalstate_multiple_exitpoints) {
    TEST_DESCRIPTION("HSM should support multiple exit points");
    /*
    @startuml
    title finalstate_multiple_exitpoints

    [*] --> P1

    state P1 {
        state A #orange
        state B
        state F1 <<exitPoint>>
        state F2 <<exitPoint>>

        [*] --> A
        A -> B: E3
        A --> F1: E1
        B --> F2: E2
    }

    F1 --> C: EXIT1
    F2 --> D: EXIT2
    @enduml
    */

    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);
    registerFinalState(AbcState::F2, AbcEvent::EXIT2);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::F1);
    registerSubstate(AbcState::P1, AbcState::F2);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E3);
    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::F2, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::C, AbcEvent::EXIT1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::EXIT2);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for state A to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    for (const auto curEvent : argEvents) {
        transition(curEvent);
        ASSERT_TRUE(waitAsyncOperation());
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {expectedState}));
}

TEST_F(ABCHsm, finalstate_blocked_final) {
    TEST_DESCRIPTION("HSM should stay in a final state if there are no matching external transitions");
    /*
    @startuml
    title finalstate_multiple_exitpoints

    [*] --> P1

    state P1 {
        state A #orange
        state F1 <<end>>
        note right of F1
            no event
        end note

        [*] --> A
        A --> F1: E1
    }

    P1 -> B: E2
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B);
    registerFinalState(AbcState::F1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::E2);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    // wait a bit to make sure there is no async exit transition
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::F1}));
}

TEST_F(ABCHsm, finalstate_blocked_exitpoint) {
    TEST_DESCRIPTION("HSM should stay in a final state (with defined event) if there are no matching external transitions");
    /*
    @startuml
    title finalstate_multiple_exitpoints

    [*] --> P1

    state P1 {
        state A #orange
        state F1 <<exitPoint>>
        note right of F1
            EXIT1 event
        end note

        [*] --> A
        A -> F1: E1
    }

    P1 --> B: E2
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::E1);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    // wait a bit to make sure there is no async exit transition
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::F1}));
}

TEST_F(ABCHsm, finalstate_transition_args) {
    TEST_DESCRIPTION("final state should forward transition arguments when rasing exit event");
    /*
    @startuml
    title finalstate_multiple_exitpoints

    [*] --> P1

    state P1 {
        state A #orange
        state F1 <<exitPoint>>
        note right of F1
            E2 event
        end note

        [*] --> A
        A -[#green,bold]> F1: **E1**
    }

    P1 -[#green,bold]-> B #LightGreen: **E2**
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const int testArg = 17;

    setInitialState(AbcState::P1);
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerFinalState(AbcState::F1, AbcEvent::E2);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::E2, this, &ABCHsm::onE2Transition);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1, testArg);
    ASSERT_TRUE(waitAsyncOperation());  // wait for B to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    ASSERT_EQ(mTransitionArgsE2.size(), 1);
    ASSERT_EQ(mTransitionArgsE2[0].toInt64(), testArg);

    ASSERT_EQ(mArgsB.size(), 1);
    ASSERT_EQ(mArgsB[0].toInt64(), testArg);
}

TEST_F(ABCHsm, finalstate_no_transition) {
    TEST_DESCRIPTION("HSM will be stuck in a final state if no one handles it's substate exit transition");
    /*
    @startuml
    title finalstate_no_transition

    [*] --> P1
    state P1 {
        state A #orange
        state F1 <<exitPoint>> #red
        note right of F1
            EXIT1 event
        end note

        [*] --> A
        A -[#green,bold]> F1: **E1**
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    hsmcpp::EventID_t failedEvent;
    std::list<hsmcpp::StateID_t> lastActiveStates;

    setInitialState(AbcState::P1);
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);

    registerFailedTransitionCallback(
        [&](const std::list<hsmcpp::StateID_t>& activeStates, const hsmcpp::EventID_t event, const VariantVector_t& args) {
            lastActiveStates = activeStates;
            failedEvent = event;
        });

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    // wait a bit to make sure there is no async exit transition
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    // state should be F1 because no one handled
    ASSERT_TRUE(compareStateLists(lastActiveStates, {AbcState::P1, AbcState::F1}));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::F1}));
    ASSERT_EQ(failedEvent, AbcEvent::EXIT1);
}

TEST_F(ABCHsm, finalstate_toplevel) {
    TEST_DESCRIPTION(
        "HSM will not generate event if final state has no parent (top level final state). "
        "Defined events will be ignored");
    /*
    @startuml
    title finalstate_toplevel

    state A #orange
    state F1 <<end>>
    note right of F1
        E2 event
    end note

    [*] -> A
    A -[#green,bold]> F1: **E1**
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    bool wasFailedEvent = false;

    registerState(AbcState::A);
    registerFinalState(AbcState::F1, AbcEvent::E2);

    registerTransition(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerFailedTransitionCallback([&](const std::list<hsmcpp::StateID_t>& activeStates,
                                         const hsmcpp::EventID_t event,
                                         const VariantVector_t& args) { wasFailedEvent = true; });

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    // wait a bit to make sure there is no async exit transition
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    // state should be B because no one handled
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::F1}));
    ASSERT_FALSE(wasFailedEvent);
}

class ParamFixtureFinalState2
    : public ABCHsm,
      public ::testing::WithParamInterface<std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t>> {};

INSTANTIATE_TEST_CASE_P(finalstate,
                        ParamFixtureFinalState2,
                        ::testing::Values(std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E1}), AbcState::C),
                                          std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E3, AbcEvent::E2}),
                                                          AbcState::C)));

TEST_P(ParamFixtureFinalState2, finalstate_exitpoint_multiple_path) {
    TEST_DESCRIPTION("Check that multiple paths to a single exit point are supported");
    /*
    @startuml
    title finalstate_exitpoint_multiple_path

    [*] --> P1
    state P1 {
        state A #orange
        state B
        state F1 <<exitPoint>>
        note right of F1
            EXIT1 event
        end note

        [*] --> A
        A -> B: E3
        A --> F1: E1
        B --> F1: E2
    }
    P1 --> C: EXIT1
    @enduml
    */

    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E3);
    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::F1, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::C, AbcEvent::EXIT1);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    for (const auto curEvent : argEvents) {
        transition(curEvent);
        ASSERT_TRUE(waitAsyncOperation());  // wait for async exit transition
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {expectedState}));
}

class ParamFixtureFinalState3
    : public ABCHsm,
      public ::testing::WithParamInterface<std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t>> {};

INSTANTIATE_TEST_CASE_P(finalstate,
                        ParamFixtureFinalState3,
                        ::testing::Values(std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E1}), AbcState::C),
                                          std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E3, AbcEvent::E2}),
                                                          AbcState::D)));

TEST_P(ParamFixtureFinalState3, finalstate_both_types) {
    TEST_DESCRIPTION("Both exitpoints and final states can exist in the same parent state");
    /*
    @startuml
    title finalstate_both_types

    [*] --> P1
    state P1 {
        state A #orange
        state B
        state F1 <<exitPoint>>
        state F2 <<end>>
        note right of F1
            EXIT1 event
        end note

        [*] --> A
        A -> B: E3
        A --> F1: E1
        B -> F2: E2
    }
    F1 --> C: EXIT1
    P1 --> D: E2
    @enduml
    */

    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);
    registerFinalState(AbcState::F2);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::B);
    registerSubstate(AbcState::P1, AbcState::F1);
    registerSubstate(AbcState::P1, AbcState::F2);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E3);
    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::F2, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::C, AbcEvent::EXIT1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::E2);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    for (const auto& curEvent : argEvents) {
        transition(curEvent);
        ASSERT_TRUE(waitAsyncOperation());  // wait for async exit transition
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {expectedState}));
}