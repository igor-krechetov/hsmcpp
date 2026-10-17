// Copyright (C) 2021 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include <chrono>
#include <thread>

#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"

#include "HsmImplTypes.hpp"

TEST(HsmImplTypes, statecallbacks_move_construct) {
    TEST_EXCLUDE("internal HsmImpl type move helper");
    TEST_DESCRIPTION("StateCallbacks move ctor transfers all three callables to the destination and empties the source");

    // *** PRECONDITIONS ***
    // Three sentinel callbacks, one per StateCallbacks member. Each flips a flag when invoked
    // so we can observe that the moved-to object owns the live callables.
    bool stateChangedFired = false;
    bool enteringFired = false;
    bool exitingFired = false;

    HsmStateChangedCallback_t onStateChanged = [&stateChangedFired](const VariantVector_t&) { stateChangedFired = true; };
    HsmStateEnterCallback_t onEntering = [&enteringFired](const VariantVector_t&) {
        enteringFired = true;
        return true;
    };
    HsmStateExitCallback_t onExiting = [&exitingFired]() {
        exitingFired = true;
        return true;
    };

    StateCallbacks src(std::move(onStateChanged), std::move(onEntering), std::move(onExiting));

    ASSERT_TRUE(static_cast<bool>(src.onStateChanged));
    ASSERT_TRUE(static_cast<bool>(src.onEntering));
    ASSERT_TRUE(static_cast<bool>(src.onExiting));

    // *** ACTIONS ***
    StateCallbacks dst(std::move(src));

    // *** VALIDATION ***
    // Destination owns all three callables and they fire the original sentinels.
    ASSERT_TRUE(static_cast<bool>(dst.onStateChanged));
    ASSERT_TRUE(static_cast<bool>(dst.onEntering));
    ASSERT_TRUE(static_cast<bool>(dst.onExiting));

    dst.onStateChanged(VariantVector_t{});
    EXPECT_TRUE(dst.onEntering(VariantVector_t{}));
    EXPECT_TRUE(dst.onExiting());

    EXPECT_TRUE(stateChangedFired);
    EXPECT_TRUE(enteringFired);
    EXPECT_TRUE(exitingFired);

    // Source callables are emptied by the move.
    EXPECT_FALSE(static_cast<bool>(src.onStateChanged));
    EXPECT_FALSE(static_cast<bool>(src.onEntering));
    EXPECT_FALSE(static_cast<bool>(src.onExiting));
}

TEST(HsmImplTypes, historyinfo_move_construct) {
    TEST_EXCLUDE("internal HsmImpl type move helper");
    TEST_DESCRIPTION("HistoryInfo move ctor transfers type/defaultTarget/previousActiveStates to the "
                     "destination and runs the source-reset branch (SHALLOW / INVALID_HSM_STATE_ID)");

    // *** PRECONDITIONS ***
    // A fully populated HistoryInfo: DEEP type, a concrete default target, a sentinel
    // transition callback, and a non-empty previousActiveStates list. The non-default
    // values let us observe both the transfer and the source reset performed by the move.
    const StateID_t defaultTarget = 7;
    bool callbackFired = false;
    HsmTransitionCallback_t onTransition = [&callbackFired](const VariantVector_t&) { callbackFired = true; };

    HistoryInfo src(HistoryType::DEEP, defaultTarget, std::move(onTransition));
    src.previousActiveStates = {11, 22, 33};

    ASSERT_EQ(HistoryType::DEEP, src.type);
    ASSERT_EQ(defaultTarget, src.defaultTarget);
    ASSERT_TRUE(static_cast<bool>(src.defaultTargetTransitionCallback));
    ASSERT_EQ(3u, src.previousActiveStates.size());

    // *** ACTIONS ***
    HistoryInfo dst(std::move(src));

    // *** VALIDATION ***
    // Destination preserves type, default target, and the previously active states list.
    EXPECT_EQ(HistoryType::DEEP, dst.type);
    EXPECT_EQ(defaultTarget, dst.defaultTarget);
    EXPECT_EQ((std::list<StateID_t>{11, 22, 33}), dst.previousActiveStates);

    // Destination owns the moved callback and it fires the original sentinel.
    ASSERT_TRUE(static_cast<bool>(dst.defaultTargetTransitionCallback));
    dst.defaultTargetTransitionCallback(VariantVector_t{});
    EXPECT_TRUE(callbackFired);

    // Source-reset branch ran: type and defaultTarget are reset to their sentinels and the
    // moved-from callback/list are emptied.
    EXPECT_EQ(HistoryType::SHALLOW, src.type);
    EXPECT_EQ(INVALID_HSM_STATE_ID, src.defaultTarget);
    EXPECT_FALSE(static_cast<bool>(src.defaultTargetTransitionCallback));
    EXPECT_TRUE(src.previousActiveStates.empty());
}

TEST(HsmImplTypes, pendingeventinfo_releaselock) {
    TEST_EXCLUDE("internal HsmImpl type lock helper");
    TEST_DESCRIPTION("PendingEventInfo::releaseLock releases the sync lock, flipping isSync() from true to false");

    // *** PRECONDITIONS ***
    // A freshly constructed PendingEventInfo is async (no lock), so isSync() is false.
    PendingEventInfo event;

    ASSERT_FALSE(event.isSync());

    // initLock() allocates the sync primitives, turning the event into a synchronous one.
    event.initLock();

    ASSERT_TRUE(event.isSync());

    // *** ACTIONS ***
    event.releaseLock();

    // *** VALIDATION ***
    // releaseLock() dropped the lock, so the event is async again and isSync() reports false.
    EXPECT_FALSE(event.isSync());
}

// Drives the synchronous transition wait() path in HsmImplTypes.cpp through the
// PUBLIC transitionSync route. Unlike the other cases in this file (which poke the
// internal helper types directly), this one needs a live HSM + dispatcher, so it
// uses a plain HierarchicalStateMachine instance created on the main thread, mirroring
// the instance-driven convention in 06_dispatchers.cpp.
TEST(HsmImplTypes, sync_wait_path_public_transitionsync) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_046");
    TEST_DESCRIPTION("a synchronous transition whose callback completes returns true after the callback has run "
                     "(caller resumed via the PendingEventInfo wait() path, public transitionSync route)");

    // *** PRECONDITIONS ***
    // A minimal A -> B HSM. B's entry callback completes normally (no blocking), so the
    // synchronous transition's wait() must be released and the caller resumed with success.
    HierarchicalStateMachine* hsm = new HierarchicalStateMachine(AbcState::A);
    std::shared_ptr<hsmcpp::IHsmEventDispatcher> dispatcher;  // must be created on main thread
    bool callbackRan = false;

    hsm->registerState(AbcState::A);
    hsm->registerState(AbcState::B, [&](const VariantVector_t& args) { callbackRan = true; });
    hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);

    ASSERT_TRUE(executeOnMainThread([&]() {
        dispatcher = CREATE_DISPATCHER();
        return hsm->initialize(dispatcher);
    }));

    // *** ACTIONS ***
    const bool transitionResult = hsm->transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION);

    // *** VALIDATION ***
    // transitionSync returned true: the wait() path observed the completion status and the
    // caller resumed. The callback having run confirms the transition actually completed
    // before the synchronous call returned (caller did not resume prematurely).
    EXPECT_TRUE(transitionResult);
    EXPECT_TRUE(callbackRan);
    EXPECT_TRUE(compareStateLists(hsm->getActiveStates(), {AbcState::B}));

    executeOnMainThread([&]() {
        delete hsm;
        dispatcher.reset();
        return true;
    });
}

// Spins a real std::thread, so it is guarded out of the FreeRTOS build per the
// suite's #ifndef TEST_HSM_FREERTOS convention.
#ifndef TEST_HSM_FREERTOS
TEST(HsmImplTypes, sync_wait_path_direct) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_046");
    TEST_DESCRIPTION("PendingEventInfo::wait() returns promptly once a worker flips the transition status via "
                     "unlock(DONE_OK), exercising the sync wait()/unlock() path directly");

    // *** PRECONDITIONS ***
    // A synchronous PendingEventInfo: initLock() allocates the sync primitives so wait()
    // actually blocks on the condition variable until the status leaves PENDING.
    PendingEventInfo event;
    event.initLock();

    ASSERT_TRUE(event.isSync());

    // *** ACTIONS ***
    // A worker flips the status to DONE_OK after a short delay. The main thread then blocks
    // in wait(); it must resume promptly once the worker notifies. We measure the elapsed
    // wait time to confirm the caller resumed on the status flip rather than on the timeout.
    const int workerDelayMs = 50;
    std::thread worker([&event, workerDelayMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(workerDelayMs));
        event.unlock(HsmEventStatus::DONE_OK);
    });

    const auto waitStart = std::chrono::steady_clock::now();
    event.wait(TIMEOUT_SYNC_TRANSITION);
    const auto waitElapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - waitStart).count();

    worker.join();

    // *** VALIDATION ***
    // wait() observed the status flip and returned well before the full timeout would elapse,
    // confirming the caller resumed on the worker's unlock(DONE_OK) rather than timing out.
    EXPECT_LT(waitElapsedMs, TIMEOUT_SYNC_TRANSITION);
}
#endif  // TEST_HSM_FREERTOS
