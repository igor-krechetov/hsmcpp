// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherBase.hpp"
#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/LockGuard.hpp"

namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherBase"

HsmEventDispatcherBase::~HsmEventDispatcherBase()
{
}

HandlerID_t HsmEventDispatcherBase::registerEventHandler(const EventHandlerFunc_t& handler)
{
    __HSM_TRACE_CALL_DEBUG__();
    HandlerID_t id = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherBase::unregisterEventHandler(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("handlerID=%d", handlerID);
    LockGuard lck(mHandlersSync);

    mEventHandlers.erase(handlerID);
}

void HsmEventDispatcherBase::emitEvent(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG__();
    LockGuard lck(mEmitSync);

    mPendingEvents.push_back(handlerID);

    // NOTE: this is not a full implementations. child classes must implement additional logic
}

HandlerID_t HsmEventDispatcherBase::registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler)
{
    __HSM_TRACE_CALL_DEBUG__();
    HandlerID_t id = getNextHandlerID();
    LockGuard lck(mHandlersSync);

    mEnqueuedEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherBase::unregisterEnqueuedEventHandler(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("handlerID=%d", handlerID);
    LockGuard lck(mHandlersSync);

    mEnqueuedEventHandlers.erase(handlerID);
}

bool HsmEventDispatcherBase::enqueueEvent(const HandlerID_t handlerID, const EventID_t event)
{
    // NOTE: should be implemented if support for transitions for interupts is needed
    return false;
}

HandlerID_t HsmEventDispatcherBase::registerTimerHandler(const TimerHandlerFunc_t& handler)
{
    const HandlerID_t newID = getNextHandlerID();

    mTimerHandlers.emplace(newID, handler);

    return newID;
}

void HsmEventDispatcherBase::unregisterTimerHandler(const HandlerID_t handlerID)
{
    auto itHandler = mTimerHandlers.find(handlerID);

    if (mTimerHandlers.end() != itHandler)
    {
        for (auto itTimer = mActiveTimers.begin(); itTimer != mActiveTimers.end();)
        {
            if (handlerID == itTimer->second.handlerID)
            {
                stopTimerImpl(itTimer->first);
                itTimer = mActiveTimers.erase(itTimer);
            }
            else
            {
                ++itTimer;
            }
        }

        mTimerHandlers.erase(itHandler);
    }
}

void HsmEventDispatcherBase::startTimer(const HandlerID_t handlerID,
                                        const TimerID_t timerID,
                                        const unsigned int intervalMs,
                                        const bool isSingleShot)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("handlerID=%d, timerID=%d, intervalMs=%d, isSingleShot=%d",
                                  handlerID, timerID, intervalMs, BOOL2INT(isSingleShot));
    if (mTimerHandlers.find(handlerID) != mTimerHandlers.end())
    {
        auto it = mActiveTimers.find(timerID);

        if (mActiveTimers.end() != it)
        {
            it->second.handlerID = handlerID;
            it->second.intervalMs = intervalMs;
            it->second.isSingleShot = isSingleShot;

            // restart timer
            stopTimerImpl(timerID);
            startTimerImpl(timerID, it->second.intervalMs, it->second.isSingleShot);
        }
        else
        {
            TimerInfo newTimer;

            newTimer.handlerID = handlerID;
            newTimer.intervalMs = intervalMs;
            newTimer.isSingleShot = isSingleShot;

            mActiveTimers.emplace(timerID, newTimer);
            startTimerImpl(timerID, intervalMs, isSingleShot);
        }
    }
}

void HsmEventDispatcherBase::restartTimer(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    auto it = mActiveTimers.find(timerID);

    if (mActiveTimers.end() != it)
    {
        stopTimerImpl(timerID);
        startTimerImpl(timerID, it->second.intervalMs, it->second.isSingleShot);
    }
}

void HsmEventDispatcherBase::stopTimer(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    auto it = mActiveTimers.find(timerID);

    __HSM_TRACE_DEBUG__("mActiveTimers=%lu", mActiveTimers.size());

    if (mActiveTimers.end() != it)
    {
        stopTimerImpl(timerID);
        mActiveTimers.erase(it);
    }
}

bool HsmEventDispatcherBase::isTimerRunning(const TimerID_t timerID)
{
    return (mActiveTimers.find(timerID) != mActiveTimers.end());
}

int HsmEventDispatcherBase::getNextHandlerID()
{
    return mNextHandlerId++;
}

void HsmEventDispatcherBase::unregisterAllEventHandlers()
{
    LockGuard lck(mHandlersSync);
    mEventHandlers.clear();
}

EnqueuedEventHandlerFunc_t HsmEventDispatcherBase::getEnqueuedEventHandlerFunc(const HandlerID_t handlerID) const
{
    EnqueuedEventHandlerFunc_t func;
    auto it = mEnqueuedEventHandlers.find(handlerID);

    if (mEnqueuedEventHandlers.end() != it)
    {
        func = it->second;
    }

    return func;
}

HsmEventDispatcherBase::TimerInfo HsmEventDispatcherBase::getTimerInfo(const TimerID_t timerID) const
{
    TimerInfo result;
    auto it = mActiveTimers.find(timerID);

    if (mActiveTimers.end() != it)
    {
        result = it->second;
    }

    return result;
}

TimerHandlerFunc_t HsmEventDispatcherBase::getTimerHandlerFunc(const HandlerID_t handlerID) const
{
    TimerHandlerFunc_t func;
    auto it = mTimerHandlers.find(handlerID);

    if (mTimerHandlers.end() != it)
    {
        func = it->second;
    }

    return func;
}

void HsmEventDispatcherBase::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot)
{
    // do nothing. must be implemented in platfrom specific dispatcher
}

void HsmEventDispatcherBase::stopTimerImpl(const TimerID_t timerID)
{
    // do nothing. must be implemented in platfrom specific dispatcher
}

bool HsmEventDispatcherBase::handleTimerEvent(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    bool restartTimer = false;

    if (INVALID_HSM_TIMER_ID != timerID)
    {
        TimerInfo curTimer = getTimerInfo(timerID);

        __HSM_TRACE_DEBUG__("curTimer.handlerID=%d", curTimer.handlerID);

        if (INVALID_HSM_DISPATCHER_HANDLER_ID != curTimer.handlerID)
        {
            TimerHandlerFunc_t timerHandler = getTimerHandlerFunc(curTimer.handlerID);

            timerHandler(timerID);

            restartTimer = (true == curTimer.isSingleShot ? false : true);
        }
    }

    return restartTimer;
}

void HsmEventDispatcherBase::dispatchPendingEvents()
{
    std::list<HandlerID_t> events;

    {
        LockGuard lck(mEmitSync);
        events = std::move(mPendingEvents);
    }

    dispatchPendingEvents(events);
}

void HsmEventDispatcherBase::dispatchPendingEvents(const std::list<HandlerID_t>& events)
{
    if (events.size() > 0)
    {
        std::map<HandlerID_t, EventHandlerFunc_t> eventHandlersCopy;

        {
            // TODO: workaround to prevent recursive lock if registerEventHandler is called from another handler
            LockGuard lck(mHandlersSync);
            eventHandlersCopy = mEventHandlers;
        }

        for (auto it = events.begin(); it != events.end(); ++it)
        {
            auto itHandler = eventHandlersCopy.find(*it);

            if (itHandler != eventHandlersCopy.end())
            {
                itHandler->second();
            }
        }
    }
}

} // namespace hsmcpp