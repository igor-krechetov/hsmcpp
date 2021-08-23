// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherBase.hpp"
#include "hsmcpp/logging.hpp"
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
    std::unique_lock<std::mutex> lck(mHandlersSync);

    mEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherBase::unregisterEventHandler(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("handlerID=%d", handlerID);
    std::unique_lock<std::mutex> lck(mHandlersSync);

    mEventHandlers.erase(handlerID);
}

void HsmEventDispatcherBase::emitEvent(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG__();
    std::unique_lock<std::mutex> lck(mEmitSync);

    mPendingEvents.push_back(handlerID);

    // NOTE: this is not a full implementations. child classes must implement additional logic
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
    std::unique_lock<std::mutex> lck(mHandlersSync);
    mEventHandlers.clear();
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

void HsmEventDispatcherBase::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    // do nothing. must be implemented in platfrom specific dispatcher
}

void HsmEventDispatcherBase::stopTimerImpl(const TimerID_t timerID) {
    // do nothing. must be implemented in platfrom specific dispatcher
}

void HsmEventDispatcherBase::dispatchPendingEvents()
{
    __HSM_TRACE_CALL_DEBUG__();

    if (mPendingEvents.size() > 0)
    {
        std::list<HandlerID_t> events;

        {
            std::unique_lock<std::mutex> lck(mEmitSync);

            events = std::move(mPendingEvents);
        }

        std::unique_lock<std::mutex> lck(mHandlersSync);

        for (auto it = events.begin(); it != events.end(); ++it)
        {
            auto itHandler = mEventHandlers.find(*it);

            if (itHandler != mEventHandlers.end())
            {
                itHandler->second();
            }
        }
    }
}

} // namespace hsmcpp