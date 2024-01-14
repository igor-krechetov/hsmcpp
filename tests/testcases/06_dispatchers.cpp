// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"

class TestHsm: public HierarchicalStateMachine {
public:
    TestHsm(const hsmcpp::StateID_t initialState, const std::string& data)
        : HierarchicalStateMachine(initialState)
    {
        mData.push_back(data);
    }

    virtual ~TestHsm() = default;

    void onState(const VariantVector_t& args) {
        auto ptr = mStateCounter.lock();

        if (ptr) {
            *ptr += 1;
        }

        // artificial delay to block HSM execution
        auto ptrM = mSyncState.lock();
        auto ptrW = mWaitState.lock();

        if (ptrM && ptrW) {
            std::unique_lock<std::mutex> lk(*ptrM);

            ptr = mPendingStateCounter.lock();

            if (ptr) {
                *ptr += 1;
            }

            ptrW->notify_all();
            ptrW->wait_for(lk, std::chrono::milliseconds(1000));
        }

        mData.push_back(mData.front() + "a");
    }

    void onStateDelayed(const VariantVector_t& args) {
        auto ptr = mStateCounter.lock();

        if (ptr) {
            *ptr += 1;
        }

        // artificial delay to block HSM execution
        std::this_thread::sleep_for(std::chrono::milliseconds(args[0].toInt64()));

        mData.push_back(mData.front() + "a");
    }

public:
    std::weak_ptr<int> mStateCounter;
    std::weak_ptr<int> mPendingStateCounter;
    std::weak_ptr<std::mutex> mSyncState;
    std::weak_ptr<std::condition_variable> mWaitState;
    std::list<std::string> mData;
};

struct TestStruct {
    char a[100] = {0};
};


TEST(dispatchers, release_sync) {
    TEST_DESCRIPTION("HSM should wait for ongoing events dispatching to stop before finishing release()");

    //-------------------------------------------
    // PRECONDITIONS
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;
    std::shared_ptr<int> callbackCounter = std::make_shared<int>(0);
    TestHsm hsm(AbcState::A, "test");
    constexpr int eventsCount = 10;
    constexpr int callbackDelayMs = 600;

    hsm.registerState(AbcState::A);
    hsm.registerState(AbcState::B, &hsm, &TestHsm::onStateDelayed);
    hsm.registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    hsm.registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

    hsm.mStateCounter = callbackCounter;

    ASSERT_TRUE(executeOnMainThread([&]() {
        // NOTE: dispatcher creation and hsm initialization must be done on the same thread
        dispatcher = CREATE_DISPATCHER();
        return hsm.initialize(dispatcher);
    }));

    //-------------------------------------------
    // ACTIONS
    hsm.transition(AbcEvent::E1, callbackDelayMs);
    // wait for HSM to enter state callback
    std::this_thread::sleep_for(std::chrono::milliseconds(callbackDelayMs / 3));

    // add more events to queue
    for (int i = 0; i < eventsCount; ++i) {
        hsm.transition(AbcEvent::E1, callbackDelayMs);
    }

    // this should block until HSM has finished dispatching ongoing event
    hsm.release();

    //-------------------------------------------
    // VALIDATION
    // NOTE: if we got here without a crash or exception then test was successful

    // need to delete dispatcher from main thread
    executeOnMainThread([&]() {
        dispatcher.reset();
        return true;
    });

    EXPECT_LT(*callbackCounter, eventsCount);
}

#ifndef TEST_HSM_FREERTOS

TEST(dispatchers, stresstest_create_destroy_hsm_later) {
    TEST_DESCRIPTION("check that it's possible to destroy HSM and disconnect from dispatcher when there are pending events");
    // The idea behild this test is to make sure that HSM is not deleted while being inside one of it's callbacks
    // To prevent such situation an enqueueAction() method of dispatcher is used to delay HSM destruction

    //-------------------------------------------
    // PRECONDITIONS
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;
    std::shared_ptr<int> callbackCounter = std::make_shared<int>(0);
    std::shared_ptr<int> pendingCallbackCounter = std::make_shared<int>(0);
    constexpr int iterationsCount = 10;
    constexpr int eventsCount = 5;
    constexpr int tempSize = 15;
    TestStruct* temp = new TestStruct();
    int objectsDeleted = 0;
    std::mutex sync;
    std::condition_variable waitCleanup;
    std::shared_ptr<std::mutex> syncState = std::make_shared<std::mutex>();
    std::shared_ptr<std::condition_variable> waitState = std::make_shared<std::condition_variable>();

    //-------------------------------------------
    // ACTIONS
    for (int i = 0; i < iterationsCount; ++i) {
        delete temp;
        TestHsm* hsm = new TestHsm(AbcState::A, "test");

        hsm->mStateCounter = callbackCounter;
        hsm->mPendingStateCounter = pendingCallbackCounter;
        hsm->mSyncState = syncState;
        hsm->mWaitState = waitState;

        hsm->registerState<TestHsm>(AbcState::A);
        hsm->registerState<TestHsm>(AbcState::B, hsm, &TestHsm::onState);
        hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
        hsm->registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

        ASSERT_TRUE(executeOnMainThread([hsm, &dispatcher]() {
            // NOTE: dispatcher creation and hsm initialization must be done on the same thread
            if (!dispatcher) {
                dispatcher = CREATE_DISPATCHER();
            }

            return hsm->initialize(dispatcher);
        }));

        hsm->transition(AbcEvent::E1);

        // wait for HSM to enter state callback
        {
            std::unique_lock<std::mutex> lk(*syncState);
            waitState->wait_for(lk, std::chrono::milliseconds(1000));
        }

        dispatcher->enqueueAction([hsm, iterationsCount, &objectsDeleted, &waitCleanup](){
            ++objectsDeleted;

            if (objectsDeleted == iterationsCount) {
                waitCleanup.notify_all();
            }
            delete hsm;
        });

        for (int j = 0; j < eventsCount; ++j) {
            hsm->transition(AbcEvent::E1);
        }

        hsm = nullptr;
        // allocated memory should have same address as deleted hsm object.
        temp = new TestStruct();
        memset(temp, 0, sizeof(TestStruct));

        // resume HSM callback
        waitState->notify_all();
    }

    // could have some data-races, but good enough for the test
    if (objectsDeleted != iterationsCount) {
        // wait for all HSM objects to be deleted
        std::unique_lock<std::mutex> lk(sync);
        waitCleanup.wait_for(lk, std::chrono::milliseconds(5000));
    }

    //-------------------------------------------
    // VALIDATION
    // NOTE: if we got here without a crash or exception then test was successful

    // need to wait to make sure we don't delete dispatcher before HSM is deleted using executeOnMainThread
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // need to delete dispatcher from main thread
    executeOnMainThread([&]() {
        dispatcher.reset();
        return true;
    });

    EXPECT_LT(*callbackCounter, iterationsCount * eventsCount);
    EXPECT_EQ(objectsDeleted, iterationsCount);
}

#endif // TEST_HSM_FREERTOS

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
