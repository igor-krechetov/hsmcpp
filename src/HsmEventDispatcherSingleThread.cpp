// Copyright (C) 2023 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherSingleThread.hpp"

#include <chrono>
#include <vector>

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"
#include "hsmcpp/os/InterruptsFreeSection.hpp"

namespace hsmcpp {

constexpr const char* HSM_TRACE_CLASS = "HsmEventDispatcherSingleThread";

HsmEventDispatcherSingleThread::HsmEventDispatcherSingleThread(const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize) {
    HSM_TRACE_CALL();
}

HsmEventDispatcherSingleThread::~HsmEventDispatcherSingleThread() {
    HSM_TRACE_CALL();

    unregisterAllEventHandlers();
}

std::shared_ptr<HsmEventDispatcherSingleThread> HsmEventDispatcherSingleThread::create(const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherSingleThread>(new HsmEventDispatcherSingleThread(eventsCacheSize),
                                                           &HsmEventDispatcherBase::handleDelete);
}

bool HsmEventDispatcherSingleThread::deleteSafe() {
    // NOTE: just delete the instance. Calling destructor from any thread is safe
    return true;
}

bool HsmEventDispatcherSingleThread::start() {
    HSM_TRACE_CALL();

    mStopDispatcher = false;

    return true;
}

void HsmEventDispatcherSingleThread::emitEvent(const HandlerID_t handlerID) {
    if (false == mStopDispatcher) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

void HsmEventDispatcherSingleThread::dispatchEvents() {
    if (false == mStopDispatcher) {
        handleTimers();
        HsmEventDispatcherBase::dispatchPendingEvents();
    }
}

void HsmEventDispatcherSingleThread::notifyDispatcherAboutEvent() {
    // NOTE: do nothing since it's a single thread implementation
}

unsigned long HsmEventDispatcherSingleThread::getTickMs() const {
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

void HsmEventDispatcherSingleThread::startTimerImpl(const TimerID_t timerID,
                                                    const unsigned int intervalMs,
                                                    const bool isSingleShot) {
    HSM_TRACE_CALL_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d", SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));

    // mRunningTimersSync is a no-op under HSM_DISABLE_THREADSAFETY (the single-threaded ship
    // config), so this costs nothing there. It becomes a real lock when thread-safety is ON, which
    // is what makes it safe to drive dispatchEvents() from a helper thread while the owning thread
    // also mutates timers. InterruptsFreeSection is kept for ISR safety (orthogonal to threading).
    CriticalSection cs(mRunningTimersSync);
    InterruptsFreeSection isr;

    auto it = mRunningTimers.find(timerID);

    if (mRunningTimers.end() == it) {
        // new timer
        RunningTimerInfo newTimer;

        newTimer.startedAt = getTickMs();
        newTimer.elapseAfter = newTimer.startedAt + intervalMs;

        mRunningTimers[timerID] = newTimer;
    } else {
        // restart timer
        it->second.startedAt = getTickMs();
        it->second.elapseAfter = it->second.startedAt + intervalMs;
    }
}

void HsmEventDispatcherSingleThread::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_ARGS("timerID=%d", SC2INT(timerID));

    CriticalSection cs(mRunningTimersSync);
    InterruptsFreeSection isr;

    mRunningTimers.erase(timerID);
}

void HsmEventDispatcherSingleThread::handleTimers() {
    // Collect the timers that have elapsed under the lock, then fire their events WITHOUT holding
    // the lock -- handleTimerEvent() runs HSM callbacks that may re-enter startTimer()/stopTimer()
    // on this same thread, which would deadlock a non-recursive lock. This mirrors the STD
    // dispatcher's split between timer bookkeeping and event firing.
    std::vector<TimerID_t> elapsed;
    const unsigned long curTime = getTickMs();

    {
        CriticalSection cs(mRunningTimersSync);
        InterruptsFreeSection isr;

        for (auto it = mRunningTimers.begin(); it != mRunningTimers.end(); ++it) {
            if (curTime >= it->second.elapseAfter) {
                elapsed.push_back(it->first);
            }
        }
    }

    for (const TimerID_t timerID : elapsed) {
        // handleTimerEvent() returns the next interval in ms (0 => timer is finished, e.g. a
        // single-shot). NOTE: the original Arduino code compared `true == handleTimerEvent(...)`,
        // which promoted the bool to 1 and only kept timers whose interval was exactly 1ms --
        // every other repeating timer was erased after its first fire. Compare against 0 instead.
        const unsigned int nextIntervalMs = handleTimerEvent(timerID);

        CriticalSection cs(mRunningTimersSync);
        InterruptsFreeSection isr;

        auto it = mRunningTimers.find(timerID);
        if (mRunningTimers.end() != it) {
            if (0u != nextIntervalMs) {
                // re-arm the repeating timer relative to its previous deadline
                const unsigned long interval = it->second.elapseAfter - it->second.startedAt;
                it->second.startedAt = getTickMs();
                it->second.elapseAfter = it->second.startedAt + interval;
            } else {
                mRunningTimers.erase(it);
            }
        }
    }
}

}  // namespace hsmcpp
