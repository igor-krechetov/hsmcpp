// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "hsm/ABCHsm.hpp"

TEST_F(ABCHsm, finalstate_simple_exitpoint) {
    // 02_simple_substate_exitpoint
    TEST_DESCRIPTION("HSM should automatically generate event when entering final state");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerFinalState(AbcState::D, AbcEvent::E3);

    registerSubstateEntryPoint(AbcState::B, AbcState::C);
    registerSubstate(AbcState::B, AbcState::D);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::B, AbcState::A, AbcEvent::E3);

    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::A);
}

TEST_F(ABCHsm, finalstate_forward_event) {
    // 01_simple_substate_final
    TEST_DESCRIPTION(
        "HSM will trigger same event as exit event "
        "if final state didn't have any event registered");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerFinalState(AbcState::D);

    registerSubstateEntryPoint(AbcState::B, AbcState::C);
    registerSubstate(AbcState::B, AbcState::D);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::B, AbcState::A, AbcEvent::E2);

    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::A);
}

class ParamFixtureFinalState1 : public ABCHsm,
                                public ::testing::WithParamInterface<std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t>> {};

INSTANTIATE_TEST_CASE_P(finalstate,
                        ParamFixtureFinalState1,
                        ::testing::Values(std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E1}), AbcState::C),
                                          std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E3, AbcEvent::E2}), AbcState::D)));

TEST_P(ParamFixtureFinalState1, finalstate_multiple_final) {
    // 03_multi_substate_final
    TEST_DESCRIPTION("HSM should support multiple path to a final state");
    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerState<ABCHsm>(AbcState::D);
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
    // sleep to allow HSM to enter substate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    for (auto curEvent : argEvents) {
        ASSERT_TRUE(transitionSync(curEvent, HSM_WAIT_INDEFINITELY));
        // wait a bit since exit transition will be async
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), expectedState);
}

TEST_P(ParamFixtureFinalState1, finalstate_multiple_exitpoints) {
    // 04_multi_substate_exitpoints
    TEST_DESCRIPTION("HSM should support multiple exit points");
    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerState<ABCHsm>(AbcState::D);
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
    // sleep to allow HSM to enter substate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    for (auto curEvent : argEvents) {
        ASSERT_TRUE(transitionSync(curEvent, HSM_WAIT_INDEFINITELY));
        // wait a bit since exit transition will be async
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), expectedState);
}

TEST_F(ABCHsm, finalstate_blocked_final) {
    // 05_blocked_substate_final
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerFinalState(AbcState::F1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::E2);

    setInitialState(AbcState::P1);
    initializeHsm();
    // sleep to allow HSM to enter substate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::F1);
}

TEST_F(ABCHsm, finalstate_blocked_exitpoint) {
    // 06_blocked_substate_exitpoint
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerFinalState(AbcState::F1, AbcEvent::EXIT1);

    registerSubstateEntryPoint(AbcState::P1, AbcState::A);
    registerSubstate(AbcState::P1, AbcState::F1);

    registerTransition<ABCHsm>(AbcState::A, AbcState::F1, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::P1, AbcState::B, AbcEvent::E1);

    setInitialState(AbcState::P1);
    initializeHsm();
    // sleep to allow HSM to enter substate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::F1);
}

TEST_F(ABCHsm, finalstate_transition_args) {
    TEST_DESCRIPTION("final state shold forward transition arguments when rasing exit event");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerFinalState(AbcState::D, AbcEvent::E3);

    registerSubstateEntryPoint(AbcState::B, AbcState::C);
    registerSubstate(AbcState::B, AbcState::D);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);
    registerTransition<ABCHsm>(AbcState::B, AbcState::A, AbcEvent::E3, this, &ABCHsm::onE3Transition);

    initializeHsm();

    const int testArg = 17;

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY, testArg));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), AbcState::A);
    ASSERT_EQ(mTransitionArgsE3.size(), 1);
    ASSERT_EQ(mTransitionArgsE3[0].toInt64(), testArg);
}

TEST_F(ABCHsm, finalstate_no_transition) {
    // 05_blocked_substate_final
    TEST_DESCRIPTION(
        "HSM will be stuck in a final state if no one handles "
        "it's substate exit transition");

    //-------------------------------------------
    // PRECONDITIONS
    hsmcpp::EventID_t failedEvent;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerFinalState(AbcState::D, AbcEvent::E3);

    registerSubstateEntryPoint(AbcState::B, AbcState::C);
    registerSubstate(AbcState::B, AbcState::D);

    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition<ABCHsm>(AbcState::C, AbcState::D, AbcEvent::E2);

    registerFailedTransitionCallback([&](const hsmcpp::EventID_t event, const VariantVector_t& args) { failedEvent = event; });

    initializeHsm();

    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    ASSERT_EQ(getLastActiveState(), AbcState::C);

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E2, HSM_WAIT_INDEFINITELY));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    // state should be D because no one handled
    ASSERT_EQ(getLastActiveState(), AbcState::D);
    ASSERT_EQ(failedEvent, AbcEvent::E3);
}

TEST_F(ABCHsm, finalstate_toplevel) {
    // 00_simple_final
    TEST_DESCRIPTION(
        "HSM will not generate event if final "
        "state has no parent (top level final state). "
        "Defined events will be ignored");

    //-------------------------------------------
    // PRECONDITIONS
    bool wasFailedEvent = false;

    registerState(AbcState::A);
    registerFinalState(AbcState::B, AbcEvent::E2);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);

    registerFailedTransitionCallback([&](const hsmcpp::EventID_t event, const VariantVector_t& args) { wasFailedEvent = true; });

    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, HSM_WAIT_INDEFINITELY));
    // wait a bit since exit transition will be async
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //-------------------------------------------
    // VALIDATION
    // state should be B because no one handled
    ASSERT_EQ(getLastActiveState(), AbcState::B);
    ASSERT_FALSE(wasFailedEvent);
}

class ParamFixtureFinalState2 : public ABCHsm,
                                public ::testing::WithParamInterface<std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t>> {};

INSTANTIATE_TEST_CASE_P(finalstate,
                        ParamFixtureFinalState2,
                        ::testing::Values(std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E1}), AbcState::C),
                                          std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E3, AbcEvent::E2}), AbcState::C)));

TEST_P(ParamFixtureFinalState2, finalstate_exitpoint_multiple_path) {
    // 07_multi_substate_single_exitpoint
    TEST_DESCRIPTION("Check that multiple paths to a single exit point are supported");
    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
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
    // sleep to allow HSM to enter substate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    for (auto curEvent : argEvents) {
        ASSERT_TRUE(transitionSync(curEvent, HSM_WAIT_INDEFINITELY));
        // wait a bit since exit transition will be async
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), expectedState);
}

class ParamFixtureFinalState3 : public ABCHsm,
                                public ::testing::WithParamInterface<std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t>> {};

INSTANTIATE_TEST_CASE_P(finalstate,
                        ParamFixtureFinalState3,
                        ::testing::Values(std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E1}), AbcState::C),
                                          std::make_tuple(std::list<hsmcpp::EventID_t>({AbcEvent::E3, AbcEvent::E2}), AbcState::D)));

TEST_P(ParamFixtureFinalState3, finalstate_both_types) {
    // 08_multi_substate_both
    // 09_multi_substate_both
    TEST_DESCRIPTION("Both exitpoints and final states can exist in the same parent state");
    std::tuple<std::list<hsmcpp::EventID_t>, hsmcpp::StateID_t> param = GetParam();
    std::list<hsmcpp::EventID_t> argEvents = std::get<0>(param);
    hsmcpp::StateID_t expectedState = std::get<1>(param);

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::P1);
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerState<ABCHsm>(AbcState::C);
    registerState<ABCHsm>(AbcState::D);
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
    // sleep to allow HSM to enter substate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(getLastActiveState(), AbcState::A);

    //-------------------------------------------
    // ACTIONS
    for (const auto& curEvent : argEvents) {
        ASSERT_TRUE(transitionSync(curEvent, HSM_WAIT_INDEFINITELY));
        // wait a bit since exit transition will be async
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(getLastActiveState(), expectedState);
}