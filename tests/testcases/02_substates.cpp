// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"
#include "hsm/TrafficLightHsm.hpp"

TEST_F(ABCHsm, substate_entrypoint) {
    TEST_DESCRIPTION("");
    /*
    @startuml
    left to right direction
    title substate_entrypoint

    A #orange -[#green,bold]-> P1: E1
    state P1 {
        [*] --> B #LightGreen
        B --> C : E1
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::C, AbcEvent::E1);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));
}

TEST_F(ABCHsm, substate_entrypoints_multiple_behavioral) {
    TEST_DESCRIPTION("hsm must support multiple exclusive conditional entry points");
    /*
    @startuml
    left to right direction
    title substate_entrypoints_multiple_behavioral

    A #orange -[#green,bold]-> P1: E1
    A --> P1: E2
    state P1 {
        [*] --> B #LightGreen: E1
        [*] --> C: E2
    }
    P1 --> A: E3
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B, AbcEvent::E1));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E2);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E3);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_entrypoints_multiple_various) {
    TEST_DESCRIPTION("if state contains both conditional and non-conditional entry points then all of them will be processed");
    /*
    @startuml
    left to right direction

    state substate_entrypoints_multiple_various {
        state "A" as 1_A #orange

        1_A --> 1_P1: E1
        1_A -[#green,bold]-> 1_P1: **E2**
        state "P1" as 1_P1 {
            state "B" as 1_B
            state "C" as 1_C
            [*] --> 1_B #LightGreen
            [*] --> 1_C #LightGreen: E2
        }
        1_P1 --> 1_A: E3
        --
        state "A" as 2_A #orange

        2_A -[#green,bold]-> 2_P1: **E1**
        2_A --> 2_P1: E2
        state "P1" as 2_P1 {
            state "B" as 2_B
            state "C" as 2_C
            [*] --> 2_B #LightGreen
            [*] --> 2_C: E2
        }
        2_P1 --> 2_A: E3
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E2);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E3);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E3, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_entrypoints_behavioral_with_conditions) {
    TEST_DESCRIPTION("entry point transitions can have both event filter and condition defined");
    /*
    @startuml
    left to right direction
    title substate_entrypoints_behavioral_with_conditions

    A #orange -[#green,bold]-> P1: **E1**
    state P1 {
        [*] --> B : <<cond==FALSE>>\nE1
        [*] --> C #LightGreen : <<cond==TRUE>>\nE1
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE(registerSubstateEntryPoint(
        AbcState::P1,
        AbcState::B,
        AbcEvent::E1,
        [](const hsmcpp::VariantVector_t&) { return true; },
        false));
    ASSERT_TRUE(registerSubstateEntryPoint(
        AbcState::P1,
        AbcState::C,
        AbcEvent::E1,
        [](const hsmcpp::VariantVector_t&) { return false; },
        false));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));
}

TEST_F(ABCHsm, substate_entrypoints_conditional) {
    TEST_DESCRIPTION("entry point transitions can have conditions callbacks defined without event filters");
    /*
    @startuml
    left to right direction
    title substate_entrypoints_conditional

    A #orange -[#green,bold]-> P1: **E1**
    state P1 {
        [*] --> B : <<cond==FALSE>>
        [*] --> C #LightGreen : <<cond==TRUE>>
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE(registerSubstateEntryPoint(
        AbcState::P1,
        AbcState::B,
        AbcEvent::INVALID,
        [](const hsmcpp::VariantVector_t&) { return true; },
        false));
    ASSERT_TRUE(registerSubstateEntryPoint(
        AbcState::P1,
        AbcState::C,
        AbcEvent::INVALID,
        [](const hsmcpp::VariantVector_t&) { return false; },
        false));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));
}

TEST_F(ABCHsm, substate_entrypoints_block_conditional_transition) {
    TEST_DESCRIPTION("if state doesnt have matching entry points for ongoing transition, then transition will be canceled");
    /*
    @startuml
    left to right direction

    state substate_entrypoints_block_conditional_transition {
        state A #orange
        A --> P1: E1
        A -[#green,bold]-> P1: **E2**
        state P1 {
            [*] -[#green,bold]-> P2: E2
            [*] --> P3: E1
            state P2 {
                [*] -[#red,bold]-> B : E1
            }
            state P3 {
                [*] --> C : E1
            }
        }
        --
        state "A" as 2_A #orange
        2_A -[#green,bold]-> 2_P1: **E1**
        2_A --> 2_P1: E2
        state "P1" as 2_P1 {
            [*] --> 2_P2: E2
            [*] -[#green,bold]-> 2_P3: E1
            state "P2" as 2_P2 {
                state "B" as 2_B
                [*] --> 2_B : E1
            }
            state "P3" as 2_P3 {
                state "C" as 2_C #LightGreen
                [*] -[#green,bold]-> 2_C : E1
            }
        }
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::P2, AbcEvent::E2));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P2, AbcState::B, AbcEvent::E1));

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::P3, AbcEvent::E1));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P3, AbcState::C, AbcEvent::E1));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    // NOTE: transition should be blocked because P2's entry point is defined with E1 event
    ASSERT_FALSE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P3, AbcState::C}));

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_entrypoints_substate) {
    TEST_DESCRIPTION("entry point of a substate could be another state with substates");
    /*
    @startuml
    left to right direction
    title substate_entrypoints_substate

    A #Orange -[#green,bold]-> P1: **E1**
    state P1 {
        [*] --> P2
        state P2 {
            [*] --> P3
            state P3 {
                [*] --> B #LightGreen
                B --> C : E1
            }
        }
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::P2));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P2, AbcState::P3));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P3, AbcState::B));
    EXPECT_TRUE(registerSubstate(AbcState::P3, AbcState::C));

    initializeHsm();

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::C, AbcEvent::E1);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::B}));
    EXPECT_EQ(mStateCounterA, 1);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterC, 0);
}

TEST_F(ABCHsm, substate_exit_single) {
    TEST_DESCRIPTION("exit state throug external parent transition");
    /*
    @startuml
    left to right direction
    title substate_exit_single

    A  --> P1: E1
    state P1 {
        [*] --> B
        B --> C #orange : E2
    }
    P1 -[#green,bold]-> A #LightGreen : E3
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, nullptr, &ABCHsm::onCExit);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::A, AbcEvent::E3, this, &ABCHsm::onE3Transition);

    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E3, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    EXPECT_EQ(mStateCounterA, 2);
    EXPECT_EQ(mStateCounterAEnter, 2);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCExit, 1);
    EXPECT_EQ(mTransitionCounterE3, 1);
}

TEST_F(ABCHsm, substate_exit_multiple_layers) {
    TEST_DESCRIPTION("Validate that exiting from multiple depth states on a top level transition is correctly handled");
    /*
    @startuml
    left to right direction

    state substate_exit_multiple_layers {
        A #Orange -[#blue,bold]-> P1: **E1**
        P1 --> A : E1
        state P1 {
            [*] -[#blue,bold]-> B #LightBlue
            B -[#magenta,bold]-> P2 : E1
            state P2 {
                [*] -[#magenta,bold]-> C #Magenta
                C -[#green,bold]-> P3 : E1
                state P3 {
                    [*] -[#green,bold]-> D #LightGreen
                }
            }
        }
        --
        state "A" as 2_A
        2_A --> 2_P1: E1
        2_P1 -[#green,bold]-> 2_A #LightGreen : E1
        state "P1" as 2_P1 {
            state "B" as 2_B
            [*] --> 2_B
            2_B --> 2_P2 : E1
            state "P2" as 2_P2 {
                state "C" as 2_C
                [*] --> 2_C
                2_C --> 2_P3 : E1
                state "P3" as 2_P3 {
                    state "D" as 2_D
                    [*] --> 2_D #Orange
                }
            }
        }
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::P2));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P2, AbcState::C));
    EXPECT_TRUE(registerSubstate(AbcState::P2, AbcState::P3));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P3, AbcState::D));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::P2, AbcEvent::E1);
    registerTransition(AbcState::C, AbcState::P3, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E2);

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::C}));

    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::P3, AbcState::D}));

    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    EXPECT_EQ(mStateCounterA, 2);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterD, 1);
}

TEST_F(ABCHsm, substate_safe_registration) {
    TEST_DESCRIPTION(
        "If HSM is compiled with safety check then it should prevent cyclic and multiple inclusions of substates. "
        "This test will fail if HSM_ENABLE_SAFE_STRUCTURE is not defined");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::A));

    //-------------------------------------------
    // ACTIONS

    // each state can be a part of only one substate
    EXPECT_FALSE(registerSubstateEntryPoint(AbcState::P2, AbcState::A));  // A is already part of P1

    // prevent recursion
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::B));
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::P2));
    EXPECT_FALSE(registerSubstateEntryPoint(AbcState::P2, AbcState::P1));  // P1 -> B2 -X P1

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P4, AbcState::C));
    EXPECT_TRUE(registerSubstate(AbcState::P4, AbcState::P3));
    EXPECT_FALSE(registerSubstateEntryPoint(AbcState::P3, AbcState::P4));  // P3 -X P4 -> P3

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P3, AbcState::P1));
    EXPECT_FALSE(registerSubstateEntryPoint(AbcState::P2, AbcState::P4));  // prevent recursion: (P4) -> P3 -> P1 -> P2 -X (P4)

    initializeHsm();

    //-------------------------------------------
    // VALIDATION
}

TEST_F(ABCHsm, substate_error_no_entrypoint) {
    TEST_DESCRIPTION("transition to a state should fail if no entry point was defined");
    /*
    @startuml
    left to right direction
    title substate_parent_as_initial

    A -[#red,bold]-> P1: E1
    state P1 {
        state B
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::B));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

TEST_F(ABCHsm, substate_parent_as_initial) {
    TEST_DESCRIPTION("when parent state is set as initial it should automatically transition into substate on startup");
    /*
    @startuml
    left to right direction
    title substate_parent_as_initial

    [*] --> P1
    state P1 {
        [*] --> P2
        state P2 {
            [*] --> B #LightGreen
        }
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::P1);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::P2));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P2, AbcState::B));

    //-------------------------------------------
    // ACTIONS
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::P2, AbcState::B}));
}