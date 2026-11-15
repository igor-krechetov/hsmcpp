// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
//
// Inline single-threaded tests for HsmEventDispatcherSingleThread.
//
// Unlike the reused suite (hsmUnitTestsSingleThread), this binary is compiled with thread-safety
// OFF (HSMBUILD_THREAD_SAFETY=OFF) -- the real shipping configuration for a genuinely
// single-threaded application. There is NO pump thread: the test itself plays the role of the
// application loop, calling dispatchEvents() inline on the one and only thread. Therefore it uses
// async transition() + explicit dispatchEvents() pumping rather than the blocking transitionSync()
// / BaseAsyncHsm rendezvous (both of which require a second thread and would deadlock here).

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "hsmcpp/HsmEventDispatcherSingleThread.hpp"
#include "hsmcpp/hsm.hpp"

using namespace hsmcpp;

namespace {

namespace State {
const StateID_t OFF = 0;
const StateID_t ON = 1;
const StateID_t DONE = 2;
}  // namespace State

namespace Event {
const EventID_t TURN_ON = 0;
const EventID_t TICK = 1;
}  // namespace Event

namespace Timer {
const TimerID_t T1 = 1;
}

// Drive the dispatcher inline until a predicate holds or a wall-clock budget elapses.
// Returns true if the predicate became true within the budget.
template <typename Predicate>
bool pumpUntil(const std::shared_ptr<HsmEventDispatcherSingleThread>& dispatcher,
               Predicate pred,
               const int budgetMs = 2000) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(budgetMs);
    while (std::chrono::steady_clock::now() < deadline) {
        dispatcher->dispatchEvents();
        if (pred()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // final pump so a timer that elapsed exactly at the deadline still gets a chance
    dispatcher->dispatchEvents();
    return pred();
}

bool inState(HierarchicalStateMachine& hsm, const StateID_t state) {
    const auto active = hsm.getActiveStates();
    return (active.size() == 1) && (active.front() == state);
}

}  // namespace

// A transition enqueued with async transition() is processed by an inline dispatchEvents() call
// on the SAME thread -- no worker thread, no synchronization needed.
TEST(SingleThreadInline, transition_processed_inline) {
    auto dispatcher = HsmEventDispatcherSingleThread::create();
    HierarchicalStateMachine hsm(State::OFF);

    hsm.registerState(State::OFF);
    hsm.registerState(State::ON);
    hsm.registerTransition(State::OFF, State::ON, Event::TURN_ON);

    ASSERT_TRUE(hsm.initialize(dispatcher));
    ASSERT_TRUE(inState(hsm, State::OFF));

    hsm.transition(Event::TURN_ON);
    // nothing has happened yet -- the application hasn't pumped the dispatcher
    ASSERT_TRUE(inState(hsm, State::OFF));

    dispatcher->dispatchEvents();
    EXPECT_TRUE(inState(hsm, State::ON));
}

// Multiple events queued before a single pump are all processed in order on one thread.
TEST(SingleThreadInline, multiple_events_single_pump) {
    auto dispatcher = HsmEventDispatcherSingleThread::create();
    HierarchicalStateMachine hsm(State::OFF);

    hsm.registerState(State::OFF);
    hsm.registerState(State::ON);
    hsm.registerState(State::DONE);
    hsm.registerTransition(State::OFF, State::ON, Event::TURN_ON);
    hsm.registerTransition(State::ON, State::DONE, Event::TICK);

    ASSERT_TRUE(hsm.initialize(dispatcher));

    hsm.transition(Event::TURN_ON);
    hsm.transition(Event::TICK);

    // one pump drains the whole queue
    ASSERT_TRUE(pumpUntil(dispatcher, [&]() { return inState(hsm, State::DONE); }));
    EXPECT_TRUE(inState(hsm, State::DONE));
}

// A transition raised from within a state-entry callback is safe (re-entrant, same thread) and
// gets processed by a subsequent pump.
TEST(SingleThreadInline, transition_from_callback) {
    auto dispatcher = HsmEventDispatcherSingleThread::create();
    HierarchicalStateMachine hsm(State::OFF);

    hsm.registerState(State::OFF);
    hsm.registerState(State::ON, [&hsm](const VariantVector_t&) {
        // raise the next event from inside the callback (single-threaded, re-entrant)
        hsm.transition(Event::TICK);
    });
    hsm.registerState(State::DONE);
    hsm.registerTransition(State::OFF, State::ON, Event::TURN_ON);
    hsm.registerTransition(State::ON, State::DONE, Event::TICK);

    ASSERT_TRUE(hsm.initialize(dispatcher));

    hsm.transition(Event::TURN_ON);
    ASSERT_TRUE(pumpUntil(dispatcher, [&]() { return inState(hsm, State::DONE); }));
    EXPECT_TRUE(inState(hsm, State::DONE));
}

// A single-shot timer fires exactly once when the application pumps across its interval.
TEST(SingleThreadInline, singleshot_timer_fires_once) {
    auto dispatcher = HsmEventDispatcherSingleThread::create();
    HierarchicalStateMachine hsm(State::OFF);

    int doneEnterCount = 0;

    hsm.registerState(State::OFF);
    hsm.registerState(State::ON);
    hsm.registerState(State::DONE, [&doneEnterCount](const VariantVector_t&) { ++doneEnterCount; });
    hsm.registerTransition(State::OFF, State::ON, Event::TURN_ON);
    hsm.registerTransition(State::ON, State::DONE, Event::TICK);

    hsm.registerTimer(Timer::T1, Event::TICK);
    hsm.registerStateAction(State::ON,
                            StateActionTrigger::ON_STATE_ENTRY,
                            StateAction::START_TIMER,
                            Timer::T1,
                            30,
                            true);  // single shot

    ASSERT_TRUE(hsm.initialize(dispatcher));

    hsm.transition(Event::TURN_ON);
    ASSERT_TRUE(pumpUntil(dispatcher, [&]() { return inState(hsm, State::DONE); }));

    // pump well past the interval to prove it does NOT fire again
    const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(120);
    while (std::chrono::steady_clock::now() < until) {
        dispatcher->dispatchEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_TRUE(inState(hsm, State::DONE));
    EXPECT_EQ(doneEnterCount, 1);
}

// A repeating timer fires multiple times while the application keeps pumping. This is the exact
// path that exposed the handleTimerEvent()-return-value bug (repeating timers were erased after
// the first fire); it guards against regression.
//
// IMPORTANT: the timer is started on entry to ON, and the repeating event (TICK) drives a
// self-transition on DONE -- NOT on ON. If TICK re-entered the timer-start state, the HSM would
// re-arm the timer via its own START_TIMER entry action on every fire, masking a broken
// handleTimers() re-arm. Keeping the fires on a different state means the dispatcher's own
// repeating-timer re-arm (handleTimers) is the ONLY thing keeping the timer alive -- which is what
// the bug broke.
TEST(SingleThreadInline, repeating_timer_fires_multiple_times) {
    auto dispatcher = HsmEventDispatcherSingleThread::create();
    HierarchicalStateMachine hsm(State::OFF);

    int tickCount = 0;

    hsm.registerState(State::OFF);
    hsm.registerState(State::ON);
    hsm.registerState(State::DONE, [&tickCount](const VariantVector_t&) { ++tickCount; });
    hsm.registerTransition(State::OFF, State::ON, Event::TURN_ON);
    hsm.registerTransition(State::ON, State::DONE, Event::TICK);
    hsm.registerSelfTransition(State::DONE, Event::TICK);

    hsm.registerTimer(Timer::T1, Event::TICK);
    hsm.registerStateAction(State::ON,
                            StateActionTrigger::ON_STATE_ENTRY,
                            StateAction::START_TIMER,
                            Timer::T1,
                            30,
                            false);  // repeating

    ASSERT_TRUE(hsm.initialize(dispatcher));

    hsm.transition(Event::TURN_ON);

    // pump for up to ~2s; a 30ms repeating timer re-armed by handleTimers() should fire several
    // times. With the bug (repeating timer erased after first fire) tickCount stops at 1.
    ASSERT_TRUE(pumpUntil(dispatcher, [&]() { return tickCount >= 3; }, 2000));
    EXPECT_GE(tickCount, 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
