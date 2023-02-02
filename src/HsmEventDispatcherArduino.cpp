// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherArduino.hpp"

#include <Arduino.h>

#include "hsmcpp/logging.hpp"

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherArduino"

// TODO: this dispatcher needs testing with interrupts (events, timers)

HsmEventDispatcherArduino::HsmEventDispatcherArduino(const size_t eventsCacheSize) {
    HSM_TRACE_CALL();
    mEnqueuedEvents.reserve(eventsCacheSize);
}

HsmEventDispatcherArduino::~HsmEventDispatcherArduino() {
    HSM_TRACE_CALL();

    unregisterAllEventHandlers();
}

bool HsmEventDispatcherArduino::start() {
    HSM_TRACE_CALL();

    mStopDispatcher = false;

    return true;
}

void HsmEventDispatcherArduino::stop() {
    HSM_TRACE_CALL();

    mStopDispatcher = true;
}

void HsmEventDispatcherArduino::emitEvent(const HandlerID_t handlerID) {
    if (false == mStopDispatcher) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

void HsmEventDispatcherSTD::notifyDispatcherAboutEvent() {
    // NOTE: do nothing since it's a single thread implementation
}

void HsmEventDispatcherArduino::dispatchEvents() {
    if (false == mStopDispatcher) {
        handleTimers();
        HsmEventDispatcherBase::dispatchPendingEvents();
    }
}

void HsmEventDispatcherArduino::startTimerImpl(const TimerID_t timerID,
                                               const unsigned int intervalMs,
                                               const bool isSingleShot) {
    HSM_TRACE_CALL_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d", SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));
    auto it = mRunningTimers.end();

    {
        // CriticalSection lck;
        it = mRunningTimers.find(timerID);
    }

    // new timer
    if (mRunningTimers.end() == it) {
        RunningTimerInfo newTimer;

        newTimer.startedAt = millis();
        newTimer.elapseAfter = newTimer.startedAt + intervalMs;

        {
            // CriticalSection lck;
            mRunningTimers.emplace(timerID, newTimer);
        }
    } else  // restart timer
    {
        it->second.startedAt = millis();
        it->second.elapseAfter = it->second.startedAt + intervalMs;
    }
}

void HsmEventDispatcherArduino::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_ARGS("timerID=%d", SC2INT(timerID));

    {
        // CriticalSection lck;
        mRunningTimers.erase(timerID);
    }
}

void HsmEventDispatcherArduino::handleTimers() {
    if (mRunningTimers.size() > 0) {
        unsigned long curTime = millis();
        auto it = mRunningTimers.begin();

        while (it != mRunningTimers.end()) {
            if (curTime >= it->second.elapseAfter) {
                if (true == handleTimerEvent(it->first)) {
                    // fast way to check timer interval without searching the map
                    it->second.elapseAfter = it->second.elapseAfter - it->second.startedAt;
                    it->second.startedAt = millis();
                    it->second.elapseAfter += it->second.startedAt;
                    ++it;
                } else {
                    it = mRunningTimers.erase(it);
                }
            } else {
                ++it;
            }
        }
    }
}

}  // namespace hsmcpp