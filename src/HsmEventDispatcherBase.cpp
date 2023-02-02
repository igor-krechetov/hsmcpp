// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherBase.hpp"

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"
#include "hsmcpp/os/LockGuard.hpp"

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherBase"

HsmEventDispatcherBase::HsmEventDispatcherBase(const size_t eventsCacheSize) {
    mEnqueuedEvents.reserve(eventsCacheSize);
}

HsmEventDispatcherBase::~HsmEventDispatcherBase() {}

HandlerID_t HsmEventDispatcherBase::registerEventHandler(const EventHandlerFunc_t& handler) {
    HSM_TRACE_CALL_DEBUG();
    HandlerID_t id = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherBase::unregisterEventHandler(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("handlerID=%d", handlerID);
    LockGuard lck(mHandlersSync);

    mEventHandlers.erase(handlerID);
}

void HsmEventDispatcherBase::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();
    LockGuard lck(mEmitSync);

    mPendingEvents.push_back(handlerID);
    notifyDispatcherAboutEvent();

    // NOTE: this is not a full implementation. child classes must implement additional logic
}

bool HsmEventDispatcherBase::enqueueEvent(const HandlerID_t handlerID, const EventID_t event) {
    bool wasAdded = false;

    if (mEnqueuedEvents.size() < mEnqueuedEvents.capacity()) {
        EnqueuedEventInfo newEvent;

        newEvent.handlerID = handlerID;
        newEvent.eventID = event;

        {
            CriticalSection cs;
            mEnqueuedEvents.push_back(newEvent);
        }

        wasAdded = true;
        notifyDispatcherAboutEvent();
    }

    return wasAdded;
}

HandlerID_t HsmEventDispatcherBase::registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler) {
    HSM_TRACE_CALL_DEBUG();
    HandlerID_t id = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mEnqueuedEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherBase::unregisterEnqueuedEventHandler(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("handlerID=%d", handlerID);
    LockGuard lck(mHandlersSync);

    mEnqueuedEventHandlers.erase(handlerID);
}

HandlerID_t HsmEventDispatcherBase::registerTimerHandler(const TimerHandlerFunc_t& handler) {
    const HandlerID_t newID = getNextHandlerID();

    mTimerHandlers.emplace(newID, handler);

    return newID;
}

void HsmEventDispatcherBase::unregisterTimerHandler(const HandlerID_t handlerID) {
    auto itHandler = mTimerHandlers.find(handlerID);

    if (mTimerHandlers.end() != itHandler) {
        for (auto itTimer = mActiveTimers.begin(); itTimer != mActiveTimers.end();) {
            if (handlerID == itTimer->second.handlerID) {
                stopTimerImpl(itTimer->first);
                itTimer = mActiveTimers.erase(itTimer);
            } else {
                ++itTimer;
            }
        }

        mTimerHandlers.erase(itHandler);
    }
}

void HsmEventDispatcherBase::startTimer(const HandlerID_t handlerID,
                                        const TimerID_t timerID,
                                        const unsigned int intervalMs,
                                        const bool isSingleShot) {
    HSM_TRACE_CALL_DEBUG_ARGS("handlerID=%d, timerID=%d, intervalMs=%d, isSingleShot=%d",
                              handlerID,
                              timerID,
                              intervalMs,
                              BOOL2INT(isSingleShot));
    if (mTimerHandlers.find(handlerID) != mTimerHandlers.end()) {
        auto it = mActiveTimers.find(timerID);

        if (mActiveTimers.end() != it) {
            it->second.handlerID = handlerID;
            it->second.intervalMs = intervalMs;
            it->second.isSingleShot = isSingleShot;

            // restart timer
            stopTimerImpl(timerID);
            startTimerImpl(timerID, it->second.intervalMs, it->second.isSingleShot);
        } else {
            TimerInfo newTimer;

            newTimer.handlerID = handlerID;
            newTimer.intervalMs = intervalMs;
            newTimer.isSingleShot = isSingleShot;

            mActiveTimers.emplace(timerID, newTimer);
            startTimerImpl(timerID, intervalMs, isSingleShot);
        }
    }
}

void HsmEventDispatcherBase::restartTimer(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    auto it = mActiveTimers.find(timerID);

    if (mActiveTimers.end() != it) {
        stopTimerImpl(timerID);
        startTimerImpl(timerID, it->second.intervalMs, it->second.isSingleShot);
    }
}

void HsmEventDispatcherBase::stopTimer(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    auto it = mActiveTimers.find(timerID);

    HSM_TRACE_DEBUG("mActiveTimers=%lu", mActiveTimers.size());

    if (mActiveTimers.end() != it) {
        stopTimerImpl(timerID);
        mActiveTimers.erase(it);
    }
}

bool HsmEventDispatcherBase::isTimerRunning(const TimerID_t timerID) {
    return (mActiveTimers.find(timerID) != mActiveTimers.end());
}

int HsmEventDispatcherBase::getNextHandlerID() {
    return mNextHandlerId++;
}

void HsmEventDispatcherBase::unregisterAllEventHandlers() {
    LockGuard lck(mHandlersSync);
    mEventHandlers.clear();
}

EnqueuedEventHandlerFunc_t HsmEventDispatcherBase::getEnqueuedEventHandlerFunc(const HandlerID_t handlerID) const {
    EnqueuedEventHandlerFunc_t func;
    auto it = mEnqueuedEventHandlers.find(handlerID);

    if (mEnqueuedEventHandlers.end() != it) {
        func = it->second;
    }

    return func;
}

HsmEventDispatcherBase::TimerInfo HsmEventDispatcherBase::getTimerInfo(const TimerID_t timerID) const {
    TimerInfo result;
    auto it = mActiveTimers.find(timerID);

    if (mActiveTimers.end() != it) {
        result = it->second;
    }

    return result;
}

TimerHandlerFunc_t HsmEventDispatcherBase::getTimerHandlerFunc(const HandlerID_t handlerID) const {
    TimerHandlerFunc_t func;
    auto it = mTimerHandlers.find(handlerID);

    if (mTimerHandlers.end() != it) {
        func = it->second;
    }

    return func;
}

void HsmEventDispatcherBase::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    // do nothing. must be implemented in platfrom specific dispatcher
}

void HsmEventDispatcherBase::stopTimerImpl(const TimerID_t timerID) {
    // do nothing. must be implemented in platfrom specific dispatcher
}

bool HsmEventDispatcherBase::handleTimerEvent(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    bool restartTimer = false;

    if (INVALID_HSM_TIMER_ID != timerID) {
        TimerInfo curTimer = getTimerInfo(timerID);

        HSM_TRACE_DEBUG("curTimer.handlerID=%d", curTimer.handlerID);

        if (INVALID_HSM_DISPATCHER_HANDLER_ID != curTimer.handlerID) {
            TimerHandlerFunc_t timerHandler = getTimerHandlerFunc(curTimer.handlerID);

            timerHandler(timerID);

            restartTimer = ((true == curTimer.isSingleShot) ? false : true);
        }
    }

    return restartTimer;
}

void HsmEventDispatcherBase::dispatchEnqueuedEvents() {
    if (false == mEnqueuedEvents.empty()) {
        HandlerID_t prevHandlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        EnqueuedEventHandlerFunc_t callback;
        std::vector<EnqueuedEventInfo> currentEvents;

        {
            CriticalSection lck;

            currentEvents = mEnqueuedEvents;
            mEnqueuedEvents.clear();
        }

        // need to traverse events in reverce order
        for (auto it = currentEvents.rbegin(); it != currentEvents.rend(); ++it) {
            if (prevHandlerID != it->handlerID) {
                callback = getEnqueuedEventHandlerFunc(it->handlerID);
                prevHandlerID = it->handlerID;
            }

            callback(it->eventID);
        }
    }
}

void HsmEventDispatcherBase::dispatchPendingEvents() {
    std::list<HandlerID_t> events;

    {
        LockGuard lck(mEmitSync);
        events = std::move(mPendingEvents);
    }

    dispatchPendingEventsImpl(events);
}

void HsmEventDispatcherBase::dispatchPendingEventsImpl(const std::list<HandlerID_t>& events) {
    dispatchEnqueuedEvents();

    if (events.size() > 0u) {
        std::map<HandlerID_t, EventHandlerFunc_t> eventHandlersCopy;

        {
            // TODO: workaround to prevent recursive lock if registerEventHandler is called from another handler
            LockGuard lck(mHandlersSync);
            eventHandlersCopy = mEventHandlers;
        }

        for (auto it = events.begin(); it != events.end(); ++it) {
            auto itHandler = eventHandlersCopy.find(*it);

            if (itHandler != eventHandlersCopy.end()) {
                itHandler->second();
            }
        }
    }
}

}  // namespace hsmcpp