// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherBase.hpp"

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"
#include "hsmcpp/os/LockGuard.hpp"

namespace hsmcpp {

constexpr const char* HSM_TRACE_CLASS = "HsmEventDispatcherBase";

HsmEventDispatcherBase::EnqueuedEventInfo::EnqueuedEventInfo(const HandlerID_t newHandlerID, const EventID_t newEventID)
    : handlerID(newHandlerID)
    , eventID(newEventID) {}

HsmEventDispatcherBase::HsmEventDispatcherBase(const size_t eventsCacheSize) {
    mEnqueuedEvents.reserve(eventsCacheSize);
}

void HsmEventDispatcherBase::handleDelete(HsmEventDispatcherBase* dispatcher) {
    if (nullptr != dispatcher) {
        dispatcher->stop();

        if (true == dispatcher->deleteSafe()) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory): this method is only used as deleter callback for std::shared_ptr
            delete dispatcher;
        }
    }
}

void HsmEventDispatcherBase::stop() {
    HSM_TRACE_CALL_DEBUG();
    mStopDispatcher = true;
    // NOTE: child classes must provide additional logic
}

HandlerID_t HsmEventDispatcherBase::registerEventHandler(const EventHandlerFunc_t& handler) {
    HSM_TRACE_CALL_DEBUG();
    HandlerID_t id = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mEventHandlers[id] = handler;

    return id;
}

void HsmEventDispatcherBase::unregisterEventHandler(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("handlerID=%d", handlerID);

    {
        LockGuard lck(mEmitSync);
        mPendingEvents.remove(handlerID);
    }
    {
        LockGuard lck(mHandlersSync);
        mEventHandlers.erase(handlerID);
    }
}

void HsmEventDispatcherBase::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();

    {
        LockGuard lck(mEmitSync);
        mPendingEvents.emplace_back(handlerID);
    }

    notifyDispatcherAboutEvent();

    // NOTE: this is not a full implementation. child classes must implement additional logic
}

bool HsmEventDispatcherBase::enqueueEvent(const HandlerID_t handlerID, const EventID_t event) {
    bool wasAdded = false;

    if (mEnqueuedEvents.size() < mEnqueuedEvents.capacity()) {
        {
            CriticalSection cs(mEnqueuedEventsSync);
            mEnqueuedEvents.emplace_back(handlerID, event);
        }

        wasAdded = true;
        notifyDispatcherAboutEvent();
    }

    return wasAdded;
}

void HsmEventDispatcherBase::enqueueAction(ActionHandlerFunc_t actionCallback) {
    HSM_TRACE_CALL_DEBUG();

    {
        LockGuard lck(mEmitSync);
        mPendingActions.emplace_back(std::move(actionCallback));
    }

    notifyDispatcherAboutEvent();
}

HandlerID_t HsmEventDispatcherBase::registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler) {
    HSM_TRACE_CALL_DEBUG();
    HandlerID_t id = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mEnqueuedEventHandlers[id] = handler;

    return id;
}

void HsmEventDispatcherBase::unregisterEnqueuedEventHandler(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("handlerID=%d", handlerID);
    LockGuard lck(mHandlersSync);

    mEnqueuedEventHandlers.erase(handlerID);
}

HandlerID_t HsmEventDispatcherBase::registerTimerHandler(const TimerHandlerFunc_t& handler) {
    const HandlerID_t newID = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mTimerHandlers[newID] = handler;

    return newID;
}

void HsmEventDispatcherBase::unregisterTimerHandler(const HandlerID_t handlerID) {
    LockGuard lck(mHandlersSync);
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
    LockGuard lck(mHandlersSync);

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

            mActiveTimers[timerID] = newTimer;
            startTimerImpl(timerID, intervalMs, isSingleShot);
        }
    }
}

void HsmEventDispatcherBase::restartTimer(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    LockGuard lck(mHandlersSync);
    auto it = mActiveTimers.find(timerID);

    if (mActiveTimers.end() != it) {
        stopTimerImpl(timerID);
        startTimerImpl(timerID, it->second.intervalMs, it->second.isSingleShot);
    }
}

void HsmEventDispatcherBase::stopTimer(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    LockGuard lck(mHandlersSync);
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
    // cppcheck-suppress misra-c2012-17.7 ; false-positive. This a function pointer, not function call.
    EnqueuedEventHandlerFunc_t func;
    auto it = mEnqueuedEventHandlers.find(handlerID);

    if (mEnqueuedEventHandlers.end() != it) {
        func = it->second;
    }

    return func;
}

TimerHandlerFunc_t HsmEventDispatcherBase::getTimerHandlerFunc(const HandlerID_t handlerID) const {
    // cppcheck-suppress misra-c2012-17.7 ; false-positive. This a function pointer, not function call.
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
        // NOTE: should lock the whole block to prevent situation when timer handler is unregistered during handler execution
        LockGuard lck(mHandlersSync);

        auto itTimer = mActiveTimers.find(timerID);

        if (mActiveTimers.end() != itTimer) {
            HSM_TRACE_DEBUG("curTimer.handlerID=%d", itTimer->second.handlerID);

            if (INVALID_HSM_DISPATCHER_HANDLER_ID != itTimer->second.handlerID) {
                TimerHandlerFunc_t timerHandler = getTimerHandlerFunc(itTimer->second.handlerID);

                restartTimer = ((true == itTimer->second.isSingleShot) ? false : true);

                // Remove singleshot timer from active list
                if (false == restartTimer) {
                    mActiveTimers.erase(itTimer);
                }

                timerHandler(timerID);
            }
        }
    }

    return restartTimer;
}

void HsmEventDispatcherBase::dispatchEnqueuedEvents() {
    if ((false == mStopDispatcher) && (false == mEnqueuedEvents.empty())) {
        HandlerID_t prevHandlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        EnqueuedEventHandlerFunc_t callback;
        std::vector<EnqueuedEventInfo> currentEvents;

        {
            CriticalSection lck(mEnqueuedEventsSync);

            // copy instead of move. we want to keep memory allocated in mEnqueuedEvents
            currentEvents = mEnqueuedEvents;
            mEnqueuedEvents.clear();
        }

        // need to traverse events in reverse order
        for (auto it = currentEvents.rbegin(); (it != currentEvents.rend()) && (false == mStopDispatcher); ++it) {
            if (prevHandlerID != it->handlerID) {
                callback = getEnqueuedEventHandlerFunc(it->handlerID);
                prevHandlerID = it->handlerID;
            }

            callback(it->eventID);
        }
    }
}

void HsmEventDispatcherBase::dispatchPendingActions() {
    if (false == mPendingActions.empty()) {
        std::list<ActionHandlerFunc_t> actionsSnapshot;

        {
            LockGuard lck(mEmitSync);
            actionsSnapshot = std::move(mPendingActions);
        }

        for (const ActionHandlerFunc_t& actionCallback : actionsSnapshot) {
            actionCallback();
        }
    }
}

void HsmEventDispatcherBase::dispatchPendingEvents() {
    std::list<HandlerID_t> events;

    dispatchPendingActions();

    if (false == mPendingEvents.empty()) {
        LockGuard lck(mEmitSync);
        events = std::move(mPendingEvents);
    }

    dispatchPendingEventsImpl(events);
}

void HsmEventDispatcherBase::dispatchPendingEventsImpl(const std::list<HandlerID_t>& events) {
    dispatchEnqueuedEvents();

    if ((false == mStopDispatcher) && (false == events.empty())) {
        std::map<HandlerID_t, EventHandlerFunc_t> eventHandlersCopy;

        {
            // TODO: workaround to prevent recursive lock if registerEventHandler/unregisterEventHandler is called from handler
            // callback
            LockGuard lck(mHandlersSync);
            eventHandlersCopy = mEventHandlers;
        }

        for (auto it = events.begin(); (it != events.end()) && (false == mStopDispatcher); ++it) {
            auto itHandler = eventHandlersCopy.find(*it);

            if (itHandler != eventHandlersCopy.end()) {
                // NOTE: if callback returns FALSE it means the handler doesn't want to process more events
                if (false == itHandler->second()) {
                    eventHandlersCopy.erase(itHandler);
                }
            }
        }
    }
}

}  // namespace hsmcpp