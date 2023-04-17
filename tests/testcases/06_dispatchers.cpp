// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"

TEST(dispatchers, stresstest_create_destroy_hsm) {
    TEST_DESCRIPTION("check that it's possible to destroy HSM and disconnect from dispatcher when there are pending events");

    //-------------------------------------------
    // PRECONDITIONS
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;

    //-------------------------------------------
    // ACTIONS
    for (int i = 0; i < 100; ++i) {
        HierarchicalStateMachine* hsm = new HierarchicalStateMachine(AbcState::A);

        hsm->registerState(AbcState::A, [](const VariantVector_t& args) {});
        hsm->registerState(AbcState::B, [](const VariantVector_t& args) {});
        hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
        hsm->registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

        ASSERT_TRUE(executeOnMainThread([hsm, &dispatcher]() {
            // NOTE: dispatcher creation and hsm initialization must be done on the same thread
            if (!dispatcher) {
                dispatcher = CREATE_DISPATCHER();
            }

            return hsm->initialize(dispatcher);
        }));

        for (int j = 0; j < 1000; ++j) {
            hsm->transition(AbcEvent::E1);
        }

        executeOnMainThread([hsm]() {
            delete hsm;
            return true;
        });
    }

    // need to wait to make sure we don't delete dispatcher before HSM is deleted using executeOnMainThread
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //-------------------------------------------
    // VALIDATION
    // NOTE: if we got here without a crash or exception then test was successful

    // need to delete dispatcher from main thread
    executeOnMainThread([&]() {
        dispatcher.reset();
        return true;
    });
}

TEST(dispatchers, stresstest_create_destroy_dispatcher) {
    TEST_DESCRIPTION("check that it's possible to destroy dispatcher when there are pending events");

    //-------------------------------------------
    // PRECONDITIONS
    const int countDispatchers = 10;
    const int countEvents = 2;
    int dispatchedEventsCount = 0;

    //-------------------------------------------
    // ACTIONS
    for (int i = 0; i < countDispatchers; ++i) {
        std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;

        executeOnMainThread([&]() {
            dispatcher = CREATE_DISPATCHER();
            return true;
        });

        HandlerID_t handler = dispatcher->registerEventHandler([&]() {
            ++dispatchedEventsCount;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return true;
        });

        executeOnMainThread([&]() {
            dispatcher->start();
            return true;
        });

        for (int j = 0; j < countEvents; ++j) {
            dispatcher->emitEvent(handler);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        executeOnMainThread([&]() {
            // NOTE: because dispatcher will process all pending events at the same time, we need to dispatcher some more events
            // from a main thread
            for (int j = 0; j < countEvents; ++j) {
                dispatcher->emitEvent(handler);
            }
            dispatcher.reset();
            return true;
        });
    }

    //-------------------------------------------
    // VALIDATION
    // NOTE: if we got here without a crash or exception then test was successful
    EXPECT_GT(dispatchedEventsCount, 0);
    EXPECT_LT(dispatchedEventsCount, countDispatchers * countEvents * 2);
}

TEST(dispatchers, destroy_without_starting) {
    TEST_DESCRIPTION("check that it's safe to destroy dispatcher without starting");

    //-------------------------------------------
    // PRECONDITIONS
    int dispatchedEventsCount = 0;
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;

    executeOnMainThread([&]() {
        dispatcher = CREATE_DISPATCHER();
        return true;
    });

    HandlerID_t handler = dispatcher->registerEventHandler([&]() {
        ++dispatchedEventsCount;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        return true;
    });

    //-------------------------------------------
    // ACTIONS
    executeOnMainThread([&]() {
        dispatcher.reset();
        return true;
    });

    //-------------------------------------------
    // VALIDATION
    // NOTE: if we got here without a crash or exception then test was successful
    EXPECT_EQ(dispatchedEventsCount, 0);
}

TEST(dispatchers, stop) {
    TEST_DESCRIPTION("check that it's possible to stop running dispatcher");

    //-------------------------------------------
    // PRECONDITIONS
    int dispatchedEventsCount = 0;
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;

    executeOnMainThread([&]() {
        dispatcher = CREATE_DISPATCHER();
        return true;
    });

    HandlerID_t handler = dispatcher->registerEventHandler([&]() {
        ++dispatchedEventsCount;
        return true;
    });

    ASSERT_TRUE(dispatcher->start());

    dispatcher->emitEvent(handler);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));  // sleep to allow dispatcher to process event
    ASSERT_EQ(dispatchedEventsCount, 1);

    //-------------------------------------------
    // ACTIONS
    dispatcher->stop();

    dispatcher->emitEvent(handler);

    executeOnMainThread([&]() {
        dispatcher.reset();
        return true;
    });

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(dispatchedEventsCount, 1);
}

TEST(dispatchers, create_hsm_from_callback) {
    TEST_DESCRIPTION("check that it's possible create a statemachine inside another HSM callback");

    //-------------------------------------------
    // PRECONDITIONS
    HierarchicalStateMachine* hsm = new HierarchicalStateMachine(AbcState::A);
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;  // must be created on main thread
    bool hsm2InitResult = false;

    hsm->registerState(AbcState::A);
    hsm->registerState(AbcState::B, [&](const VariantVector_t& args) {
        HierarchicalStateMachine hsm2(AbcState::A);
        hsm2InitResult = hsm2.initialize(dispatcher);
    });
    hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);

    ASSERT_TRUE(executeOnMainThread([&]() {
        dispatcher = CREATE_DISPATCHER();
        return hsm->initialize(dispatcher);
    }));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(hsm->transitionSync(AbcEvent::E1, 500));

    //-------------------------------------------
    // VALIDATION
    // NOTE: test is ok if there is no dead-lock or unexpected behaviour in hsm2.initialize()
    executeOnMainThread([&]() {
        delete hsm;
        dispatcher.reset();

        return true;
    });

    EXPECT_TRUE(hsm2InitResult);
}

TEST(dispatchers, instance_ownership) {
    TEST_DESCRIPTION("HSM should not own instance of dispatcher and should not crash if this instance is deleted");

    //-------------------------------------------
    // PRECONDITIONS
    HierarchicalStateMachine* hsm = new HierarchicalStateMachine(AbcState::A);
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;  // must be created on main thread
    int callbackCount = 0;

    hsm->registerState(AbcState::A);
    hsm->registerState(AbcState::B, [&](const VariantVector_t& args) { ++callbackCount; });
    hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);

    ASSERT_TRUE(executeOnMainThread([&]() {
        dispatcher = CREATE_DISPATCHER();
        return hsm->initialize(dispatcher);
    }));

    //-------------------------------------------
    // ACTIONS
    executeOnMainThread([&]() {
        dispatcher.reset();
        return true;
    });

    //-------------------------------------------
    // VALIDATION
    ASSERT_FALSE(hsm->transitionSync(AbcEvent::E1, 500));

    EXPECT_EQ(callbackCount, 0);
}

// TODO: glib/glibmm dispatchers with custom context
