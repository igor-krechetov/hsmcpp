// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "hsm/ABCHsm.hpp"

class ParamFixtureStateActions1 : public ABCHsm,
                                  public ::testing::WithParamInterface<std::tuple<hsmcpp::StateActionTrigger,
                                                                                  hsmcpp::StateActionTrigger>> {};

INSTANTIATE_TEST_CASE_P(state_actions,
                        ParamFixtureStateActions1,
                        ::testing::Values(std::make_tuple(hsmcpp::StateActionTrigger::ON_STATE_ENTRY,
                                                          hsmcpp::StateActionTrigger::ON_STATE_EXIT),
                                          std::make_tuple(hsmcpp::StateActionTrigger::ON_STATE_ENTRY,
                                                          hsmcpp::StateActionTrigger::ON_STATE_ENTRY),
                                          std::make_tuple(hsmcpp::StateActionTrigger::ON_STATE_EXIT,
                                                          hsmcpp::StateActionTrigger::ON_STATE_EXIT)));

TEST_P(ParamFixtureStateActions1, state_actions_simple) {
    TEST_DESCRIPTION("Validate that one or multiple state actions can be executed on state entry and exit");
    /*
    @startuml
    title state_actions_simple

    state A #orange
    state B: onEntry: transition(E2)\nonXXX: transition(E3)\nonXXX: transition(E4)
    state C
    state D
    state E

    [*] -> A
    A -[#green,bold]-> B: **E1**
    B -[#green,bold]> C: **E2**
    C -[#green,bold]> D: **E3**
    D -[#green,bold]> E #LightGreen: **E4**
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    const hsmcpp::StateActionTrigger trigger1 = std::get<0>(GetParam());
    const hsmcpp::StateActionTrigger trigger2 = std::get<1>(GetParam());

    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onSyncA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onSyncB);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);
    registerState<ABCHsm>(AbcState::D, this, &ABCHsm::onSyncD);
    registerState<ABCHsm>(AbcState::E, this, &ABCHsm::onSyncE);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E3);
    registerTransition<ABCHsm>(AbcState::D, AbcState::E, AbcEvent::E4);

    registerStateAction(AbcState::B,
                        hsmcpp::StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::TRANSITION,
                        static_cast<int>(AbcEvent::E2));
    registerStateAction(AbcState::B,
                        trigger1,
                        StateAction::TRANSITION,
                        static_cast<int>(AbcEvent::E3));
    registerStateAction(AbcState::B,
                        trigger2,
                        StateAction::TRANSITION,
                        static_cast<int>(AbcEvent::E4));
    setSyncMode(false);// to avoid being blocked in A state
    initializeHsm();
    setSyncMode(true);

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation(false));// wait for state B to activate
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    unblockNextStep();

    ASSERT_TRUE(waitAsyncOperation(false));// wait for state C to activate
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    unblockNextStep();

    ASSERT_TRUE(waitAsyncOperation(false));// wait for state D to activate
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::D}));
    unblockNextStep();

    ASSERT_TRUE(waitAsyncOperation());// wait for state E to activate

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(compareStateLists(getActiveStates(), {AbcState::E}));
}

TEST_F(ABCHsm, state_actions_args) {
    TEST_DESCRIPTION("transition actions should support arguments");
    /*
    @startuml
    title state_actions_args

    state A #orange
    state B: onEntry: transition(E2, 123, "text")
    state C

    [*] -> A
    A -[#green,bold]-> B: **E1**
    B -[#green,bold]-> C #LightGreen: **E2**
    @enduml
    */

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C, this, &ABCHsm::onSyncC);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::B, AbcState::C, AbcEvent::E2);

    registerStateAction(AbcState::B,
                        hsmcpp::StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::TRANSITION,
                        static_cast<int>(AbcEvent::E2),
                        123,
                        "string arg");
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    transition(AbcEvent::E1);
    ASSERT_TRUE(waitAsyncOperation());// wait for state C to activate

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::C}));
    ASSERT_EQ(mArgsC.size(), 2);
    ASSERT_TRUE(mArgsC[0].isNumeric());
    ASSERT_TRUE(mArgsC[1].isString());
    ASSERT_EQ(mArgsC[0].toInt64(), 123);
    ASSERT_EQ(mArgsC[1].toString(), "string arg");
}
