// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsm/ABCHsm.hpp"

// Notation
// B        : regular state
// ->       : transition
// B + C    : transition to 2 states at the same time
// *A       : initial active state
// [...]    : parent state
// #C       : entry point
// !#C      : entry point with a false condition
// {Cx}     : transition from state C was blocked

TEST_F(ABCHsm, parallel_transition_01) {
    TEST_DESCRIPTION("*A -> B + C");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_02) {
    TEST_DESCRIPTION("*A -> B + [#C]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::P1, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_05) {
    TEST_DESCRIPTION("*A -> B + [C]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, parallel_transition_03) {
    TEST_DESCRIPTION("*A -> B + [!#C, #D]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::D));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::P1, AbcState::D}));
}

TEST_F(ABCHsm, parallel_transition_04) {
    TEST_DESCRIPTION("Parallel outer transitions");
    /*
    @startuml
    title parallel_transition_04

    [*] --> P1

    state P1 {
        [*] -> A #Orange
    }

    P1 --> B #LightGreen: E1
    P1 --> C #LightGreen: E1

    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::A));

    registerTransition(AbcState::P1, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E1);

    setInitialState(AbcState::P1);
    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A state to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for B state to activate
    ASSERT_TRUE(waitAsyncOperation());  // wait for C state to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_outer) {
    TEST_DESCRIPTION("It should be possible to transition out of a parent state even if it has multiple active substates");

    /*
    @startuml
    title parallel_transition_outer

    state steps {
        state "P1" as 1_P1 {
            state "A" as 1_A #orange
            state "B" as 1_B #orange

            [*] --> 1_A
            [*] --> 1_B
        }
        state "C" as 1_C #LightGreen

        1_P1 -right[#green,bold]-> 1_C: E1

        --
        state "P1" as 2_P1 {
            state "A" as 2_A
            state "B" as 2_B

            [*] --> 2_A
            [*] --> 2_B
        }
        state "C" as 2_C #orange

        2_P1 -right-> 2_C: E1
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::P1);

    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::A));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));

    registerTransition(AbcState::P1, AbcState::C, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(waitAsyncOperation());  // wait for B to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A, AbcState::B}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_06) {
    TEST_DESCRIPTION("*A -> B + [!#C]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C, AbcEvent::E2));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, parallel_transition_07) {
    TEST_DESCRIPTION("*A -> B + [#[C]]");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::P2));
    EXPECT_TRUE(registerSubstate(AbcState::P2, AbcState::C));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
}

TEST_F(ABCHsm, parallel_transition_08) {
    TEST_DESCRIPTION("*A -> B + C -> A: check that transitions are not applied recursively");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);

    // since transitions are applied one by one we want to make sure that HSM will not transition to A while doing (*A -> B + C)
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);
    registerTransition(AbcState::C, AbcState::A, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_09) {
    TEST_DESCRIPTION("A -> [*#B + *#C]>e2  -e2-> D");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD, &ABCHsm::onDEnter, &ABCHsm::onDExit);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::E2, this, &ABCHsm::onE2Transition);
    registerSelfTransition<ABCHsm>(AbcState::P1,
                                   AbcEvent::E2,
                                   ABCHsm::TransitionType::INTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onSelfTransition);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExit, 1);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);
    EXPECT_EQ(mStateCounterD, 1);
    EXPECT_EQ(mStateCounterDEnter, 1);
    EXPECT_EQ(mStateCounterDExit, 0);

    EXPECT_EQ(mTransitionCounterE2, 1);
    EXPECT_EQ(mTransitionCounterSelf, 1);
}

TEST_F(ABCHsm, parallel_transition_10_internal_priority) {
    TEST_DESCRIPTION("[*A -e1-> B + C] -e1-> D: internal transitions have priority over external ones");
    /*
    @startuml
    title parallel_transition_04

    [*] --> P1

    state P1 {
        [*] --> A #Orange
        A --> B #LightGreen: E1
        A --> C #LightGreen: E1
    }

    P1 -> D: E1

    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    setInitialState(AbcState::P1);
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::A));
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::B));
    EXPECT_TRUE(registerSubstate(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);
    registerTransition(AbcState::P1, AbcState::D, AbcEvent::E1);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A state to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for B state to activate
    ASSERT_TRUE(waitAsyncOperation());  // wait for C state to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));
}

TEST_F(ABCHsm, parallel_transition_canceled_01) {
    TEST_DESCRIPTION("A -> [*#B + *#C] -> D {Cx}");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExitCancel);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D);

    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    ASSERT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, AbcEvent::E2, this, &ABCHsm::onE2Transition);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_FALSE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::C}));
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExitCancel, 1);
    EXPECT_EQ(mStateCounterC, 2);
    EXPECT_EQ(mStateCounterCEnter, 2);
    EXPECT_EQ(mStateCounterCExit, 1);

    EXPECT_EQ(mTransitionCounterE2, 0);
}

TEST_F(ABCHsm, parallel_transition_canceled_02) {
    TEST_DESCRIPTION("A -> / *B -> D + xE / *C -> F /");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onD, &ABCHsm::onDEnter, &ABCHsm::onDExit);
    registerState<ABCHsm>(AbcState::E, this, &ABCHsm::onE, &ABCHsm::onEEnterCancel, &ABCHsm::onEExit);
    registerState<ABCHsm>(AbcState::F, this, &ABCHsm::onF, &ABCHsm::onFEnter, &ABCHsm::onFExit);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);

    registerTransition(AbcState::B, AbcState::D, AbcEvent::E2);
    registerTransition(AbcState::B, AbcState::E, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::F, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::D, AbcState::F, AbcState::B}));
    EXPECT_EQ(mStateCounterB, 2);
    EXPECT_EQ(mStateCounterBEnter, 2);
    EXPECT_EQ(mStateCounterBExit, 1);

    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);

    EXPECT_EQ(mStateCounterD, 1);
    EXPECT_EQ(mStateCounterDEnter, 1);
    EXPECT_EQ(mStateCounterDExit, 0);

    EXPECT_EQ(mStateCounterE, 0);
    EXPECT_EQ(mStateCounterEEnterCancel, 1);
    EXPECT_EQ(mStateCounterEExit, 0);

    EXPECT_EQ(mStateCounterF, 1);
    EXPECT_EQ(mStateCounterFEnter, 1);
    EXPECT_EQ(mStateCounterFExit, 0);
}

TEST_F(ABCHsm, parallel_transition_mult2one_01) {
    TEST_DESCRIPTION("A -> *B + *C -> A");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::C, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E2);
    registerTransition(AbcState::C, AbcState::A, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

TEST_F(ABCHsm, parallel_transition_mult2one_02) {
    TEST_DESCRIPTION("A -> *B + [*C] -> A");

    /*
    @startuml
    left to right direction

    state parallel_transition_mult2one_02 {
        state "A" as 2_A #LightGreen
        state "B" as 2_B #orange
        state "P1" as 2_P1 {
            state "C" as 2_C #orange
        }

        2_A --> 2_B: E1
        2_A --> 2_P1: E1
        2_B -[#green,bold]-> 2_A: E2
        2_P1 -[#green,bold]-> 2_A: E2
        --
        state A #orange
        state B
        state "P1" as P1 {
            state C
        }

        A --> B: E1
        A --> P1: E1
        B --> A: E2
        P1 --> A: E2
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onC);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::C));

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::A, AbcState::P1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E2);
    registerTransition(AbcState::P1, AbcState::A, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::P1, AbcState::C}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));
}

class ParamFixtureParallel1 : public ABCHsm, public ::testing::WithParamInterface<std::vector<hsmcpp::EventID_t>> {};

INSTANTIATE_TEST_CASE_P(parallel,
                        ParamFixtureParallel1,
                        ::testing::ValuesIn(std::vector<std::vector<hsmcpp::EventID_t>>{{AbcEvent::E3, AbcEvent::E3},
                                                                                        {AbcEvent::INVALID, AbcEvent::E2}}));

TEST_P(ParamFixtureParallel1, parallel_substate_final) {
    TEST_DESCRIPTION("Final HSM should wait for all parallel states to finish before exiting final state");

    /*
    @startuml

    state parallel_substate_final {
        state "P1" as 1_P1 {
            state "A" as 1_A #orange
            state "B" as 1_B #orange

            [*] -> 1_A
            [*] -up-> 1_B

            1_A -[#green,bold]-> [*] : **E1**
            1_B -> [*] : E2
            1_B -> 1_A: E3
        }

        state "C" as 1_C
        1_P1 -right-> 1_C: E3

        --
        state "P1" as 2_P1 {
            state "A" as 2_A
            state "B" as 2_B #orange

            [*] -> 2_A
            [*] -up-> 2_B

            2_A -> [*] : E1
            2_B -[#green,bold]-> [*] : **E2**
            2_B -> 2_A: E3
        }

        state "C" as 2_C
        2_P1 -right-> 2_C: E3

        --
        state "P1" as 3_P1 {
            state "A" as 3_A
            state "B" as 3_B

            [*] -> 3_A
            [*] -up-> 3_B

            3_A -> [*] : E1
            3_B -> [*] : E2
            3_B -> 3_A: E3
        }

        state "C" as 3_C #LightGreen
        3_P1 -right[#green,bold]-> 3_C: **E3**
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const hsmcpp::EventID_t eventFinalState = GetParam()[0];
    const hsmcpp::EventID_t eventTransition = GetParam()[1];

    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerFinalState<ABCHsm>(AbcState::F1, eventFinalState, this, &ABCHsm::onSyncF1);

    setInitialState(AbcState::P1);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::A));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::F1, AbcEvent::E2);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E3);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::C, eventTransition, this, &ABCHsm::onSyncE3Transition);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(waitAsyncOperation(300, false));  // wait for B to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A, AbcState::B}));
    ASSERT_EQ(mStateCounterA, 1);
    ASSERT_EQ(mStateCounterB, 1);
    unblockNextStep();

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for F1 to activate
    ASSERT_EQ(mStateCounterF1, 1);
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::F1}));

    transition(AbcEvent::E2);
    ASSERT_TRUE(waitAsyncOperation());  // wait for E3 transition
    ASSERT_EQ(mTransitionCounterE3, 1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for C to activate

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    EXPECT_EQ(mStateCounterF1, 1);
    EXPECT_EQ(mStateCounterC, 1);
}

class ParamFixtureParallel2 : public ABCHsm, public ::testing::WithParamInterface<std::vector<hsmcpp::EventID_t>> {};

INSTANTIATE_TEST_CASE_P(parallel,
                        ParamFixtureParallel2,
                        ::testing::ValuesIn(std::vector<std::vector<hsmcpp::EventID_t>>{
                            {AbcEvent::E3, AbcEvent::E4, AbcEvent::E3, AbcEvent::E4},
                            {AbcEvent::E3, AbcEvent::INVALID, AbcEvent::E3, AbcEvent::E2},
                            {AbcEvent::INVALID, AbcEvent::INVALID, AbcEvent::E1, AbcEvent::E2}}));

TEST_P(ParamFixtureParallel2, parallel_substate_final_multiple) {
    TEST_DESCRIPTION(
        "If there are multiple final states registered, HSM will wait for all of child states to "
        "deactivate, but will only process the last activated final state");
    /*
    @startuml
    left to right direction
    title "parallel_substate_final_multiple step 1"

    state P1 {
        state A #orange
        state B #orange
        state exitF1 <<exitPoint>> #LightGreen
        state exitF2 <<exitPoint>>

        [*] --> A
        [*] --> B

        A -[#green,bold]-> exitF1 : **E1**
        B --> exitF2 : E2
    }

    exitF1 --> C: E3
    exitF2 --> D: E4
    @enduml
    */

    /*
    @startuml
    left to right direction
    title "parallel_substate_final_multiple step 2"

    state P1 {
        state A
        state B #orange
        state exitF1 <<exitPoint>> #orange
        state exitF2 <<exitPoint>> #LightGreen

        [*] --> A
        [*] --> B

        A --> exitF1 : E1
        B -[#green,bold]-> exitF2 : **E2**
    }

    exitF1 --> C: E3
    exitF2 --> D: E4
    @enduml
    */

    /*
     @startuml
     left to right direction
     title "parallel_substate_final_multiple step 3"

     state P1 {
         state A
         state B
         state exitF1 <<exitPoint>> #orange
         state exitF2 <<exitPoint>> #orange

         [*] --> A
         [*] --> B

         A --> exitF1 : E1
         B --> exitF2 : E2
     }

     exitF1 --> C: E3
     exitF2 -[#green,bold]-> D #LightGreen: **E4**
     @enduml
     */

    //-------------------------------------------
    // PRECONDITIONS
    const hsmcpp::EventID_t eventFinalState1 = GetParam()[0];
    const hsmcpp::EventID_t eventFinalState2 = GetParam()[1];
    const hsmcpp::EventID_t eventTransition1 = GetParam()[2];
    const hsmcpp::EventID_t eventTransition2 = GetParam()[3];

    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);
    registerFinalState<ABCHsm>(AbcState::F1, eventFinalState1, this, &ABCHsm::onSyncF1);
    registerFinalState<ABCHsm>(AbcState::F2, eventFinalState2, this, &ABCHsm::onSyncF2);

    setInitialState(AbcState::P1);

    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::A));
    EXPECT_TRUE(registerSubstateEntryPoint(AbcState::P1, AbcState::B));
    registerSubstate(AbcState::P1, AbcState::F1);
    registerSubstate(AbcState::P1, AbcState::F2);

    registerTransition(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::F2, AbcEvent::E2, this, &ABCHsm::onSyncE2Transition);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::C, eventTransition1, this, &ABCHsm::onSyncE3Transition);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::D, eventTransition2, this, &ABCHsm::onSyncE4Transition);

    initializeHsm();
    ASSERT_TRUE(waitAsyncOperation());  // wait for A to activate
    ASSERT_TRUE(waitAsyncOperation());  // wait for B to activate

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::A, AbcState::B}));
    ASSERT_EQ(mStateCounterA, 1);
    ASSERT_EQ(mStateCounterB, 1);

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());  // wait for F1 to activate
    ASSERT_EQ(mStateCounterF1, 1);
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::B, AbcState::F1}));

    transition(AbcEvent::E2);
    ASSERT_TRUE(waitAsyncOperation());  // wait for E2 transition
    ASSERT_EQ(mTransitionCounterE2, 1);
    ASSERT_TRUE(waitAsyncOperation(false));  // wait for F2 to activate
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::P1, AbcState::F1, AbcState::F2}));
    unblockNextStep();

    ASSERT_TRUE(waitAsyncOperation());  // wait for E4 transition
    ASSERT_TRUE(waitAsyncOperation());  // wait for D to activate

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));

    EXPECT_EQ(mTransitionCounterE3, 0);
    EXPECT_EQ(mTransitionCounterE4, 1);

    EXPECT_EQ(mStateCounterF1, 1);
    EXPECT_EQ(mStateCounterF2, 1);

    EXPECT_EQ(mStateCounterC, 0);
    EXPECT_EQ(mStateCounterD, 1);
}

TEST_F(ABCHsm, parallel_callbacks) {
    TEST_DESCRIPTION("*A -> B + C -> A: check that all callbacks are correctly executed");
    /*
    @startuml
    left to right direction

    state parallel_callbacks {
        state A #orange : **onAEnter**\n**onA**\n**onAExit**
        state B #LightGreen : **onBEnter**\n**onB**\nonBExit
        state C #LightGreen : **onCEnter**\n**onC**\nonCExit

        A -[#green,bold]-> B: E1
        A -[#green,bold]-> C: E1
        B --> A: E2
        C --> A: E2
        --
        state "A" as 2_A #LightGreen : **onAEnter**\n**onA**\nonAExit
        state "B" as 2_B #orange : onBEnter\nonB\n**onBExit**
        state "C" as 2_C #orange : onCEnter\nonC\n**onCExit**

        2_A --> 2_B: E1
        2_A --> 2_C: E1
        2_B -[#green,bold]-> 2_A: E2
        2_C -[#green,bold]-> 2_A: E2
    }
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onC, &ABCHsm::onCEnter, &ABCHsm::onCExit);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1, this, &ABCHsm::onE1Transition);
    registerTransition<ABCHsm>(AbcState::A, AbcState::C, AbcEvent::E1, this, &ABCHsm::onE1Transition);
    registerTransition<ABCHsm>(AbcState::B, AbcState::A, AbcEvent::E2, this, &ABCHsm::onE2Transition);
    registerTransition<ABCHsm>(AbcState::C, AbcState::A, AbcEvent::E2, this, &ABCHsm::onE2Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B, AbcState::C}));
    ASSERT_EQ(mStateCounterA, 1);
    ASSERT_EQ(mStateCounterAEnter, 1);
    ASSERT_EQ(mStateCounterAExit, 1);
    ASSERT_EQ(mTransitionCounterE1, 2);

    ASSERT_TRUE(transitionSync(AbcEvent::E2, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mStateCounterA, 2);
    EXPECT_EQ(mStateCounterAEnter, 2);
    EXPECT_EQ(mStateCounterAExit, 1);
    EXPECT_EQ(mStateCounterB, 1);
    EXPECT_EQ(mStateCounterBEnter, 1);
    EXPECT_EQ(mStateCounterBExit, 1);
    EXPECT_EQ(mStateCounterC, 1);
    EXPECT_EQ(mStateCounterCEnter, 1);
    EXPECT_EQ(mStateCounterCExit, 1);

    ASSERT_EQ(mTransitionCounterE1, 2);
    ASSERT_EQ(mTransitionCounterE2, 2);
}

TEST_F(ABCHsm, parallel_selftransition) {
    TEST_DESCRIPTION(
        "*A -> A + B: when we have both a regular and self-transition self-transition will"
        " be excecuted first before exiting state");
    /*
    @startuml
    left to right direction
    title parallel_callbacks

    state A #orange : **onAEnter**\n**onA**\n**onAExit**
    state B #LightGreen : **onBEnter**\n**onB**\nonBExit

    A -[#green,bold]-> A: **E1**
    A -[#green,bold]-> B: **E1**
    @enduml
    */
    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);

    registerSelfTransition<ABCHsm>(AbcState::A,
                                   AbcEvent::E1,
                                   ABCHsm::TransitionType::INTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onE1Transition);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1, this, &ABCHsm::onE1Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mStateCounterA, 1);
    EXPECT_EQ(mStateCounterAEnter, 1);
    EXPECT_EQ(mStateCounterAExit, 1);
    EXPECT_EQ(mTransitionCounterE1, 2);
}

TEST_F(ABCHsm, parallel_selftransition_multiple) {
    TEST_DESCRIPTION("*A -> A + A: check that multiple self transitions are correctly handled");
    /*
    @startuml
    left to right direction
    title parallel_selftransition_multiple

    state A #orange : **onAEnter**\n**onA**\nonAExit
    state B : onBEnter\nonB\nonBExit

    A -[#green,bold]-> A: E1 <<onE1Transition>>
    A -[#green,bold]-> A: E1 <<onE2Transition>>
    A --> B: E3
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA, &ABCHsm::onAEnter, &ABCHsm::onAExit);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB, &ABCHsm::onBEnter, &ABCHsm::onBExit);

    registerSelfTransition<ABCHsm>(AbcState::A,
                                   AbcEvent::E1,
                                   ABCHsm::TransitionType::INTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onE1Transition);
    registerSelfTransition<ABCHsm>(AbcState::A,
                                   AbcEvent::E1,
                                   ABCHsm::TransitionType::INTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onE2Transition);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E3, this, &ABCHsm::onE3Transition);

    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(mStateCounterA, 1);
    EXPECT_EQ(mStateCounterAEnter, 1);
    EXPECT_EQ(mStateCounterAExit, 0);
    EXPECT_EQ(mTransitionCounterE1, 1);
    EXPECT_EQ(mTransitionCounterE2, 1);
    EXPECT_EQ(mTransitionCounterE3, 0);
}