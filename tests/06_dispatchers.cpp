// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"

// NOTE: Qt doesn't support cancelation of already posted events. So deleting dispatcher before
//       all events are processed will result in a crash.
#ifndef TEST_HSM_QT
TEST(dispatchers, stresstest_create_destroy) {
    TEST_DESCRIPTION("check that it's possible to destroy HSM and disconnect from dispatcher when there are events pending");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS

    for (int i = 0; i < 10; ++i) {
        HierarchicalStateMachine* hsm = new HierarchicalStateMachine(AbcState::A);

        hsm->registerState(AbcState::A, [](const VariantVector_t& args) {});
        hsm->registerState(AbcState::B, [](const VariantVector_t& args) {});
        hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
        hsm->registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

        ASSERT_TRUE(executeOnMainThread([&]() { return hsm->initialize(CREATE_DISPATCHER()); }));

        for (int j = 0; j < 100; ++j) {
            hsm->transition(AbcEvent::E1);
        }

        executeOnMainThread([&]() {
            delete hsm;
            return true;
        });
    }

    //-------------------------------------------
    // VALIDATION
}

#endif  // TEST_HSM_QT

TEST(dispatchers, create_hsm_from_callback) {
    TEST_DESCRIPTION("check that it's possible create a statemachine inside another HSM callback");

    //-------------------------------------------
    // PRECONDITIONS
    HierarchicalStateMachine* hsm = new HierarchicalStateMachine(AbcState::A);
    auto dispatcher = CREATE_DISPATCHER();

    hsm->registerState(AbcState::A);
    hsm->registerState(AbcState::B, [&](const VariantVector_t& args) {
        HierarchicalStateMachine hsm2(AbcState::A);
        hsm2.initialize(dispatcher);
    });
    hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);

    ASSERT_TRUE(executeOnMainThread([&]() { return hsm->initialize(dispatcher); }));

    //-------------------------------------------
    // ACTIONS
    hsm->transition(AbcEvent::E1);

    //-------------------------------------------
    // VALIDATION
    // NOTE: test is ok if there is no dead-lock or unexpected behaviour in hsm2.initialize()
    dispatcher.reset();

    executeOnMainThread([&]() {
        delete hsm;
        return true;
    });
}