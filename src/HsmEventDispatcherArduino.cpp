// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherArduino.hpp"
#include "hsmcpp/logging.hpp"
#include <Arduino.h>

namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherArduino"

// TODO: this dispatcher needs testing with interrupts (events, timers)
// TODO: implement critical section for Arduino

HsmEventDispatcherArduino::HsmEventDispatcherArduino(const size_t eventsCacheSize)
{
    __HSM_TRACE_CALL__();
    mEnqueuedEvents.reserve(eventsCacheSize);
}

HsmEventDispatcherArduino::~HsmEventDispatcherArduino()
{
    __HSM_TRACE_CALL__();

    unregisterAllEventHandlers();
}

bool HsmEventDispatcherArduino::start()
{
    __HSM_TRACE_CALL__();

    mStopDispatcher = false;

    return true;
}

void HsmEventDispatcherArduino::stop()
{
    __HSM_TRACE_CALL__();

    mStopDispatcher = true;
}

void HsmEventDispatcherArduino::emitEvent(const HandlerID_t handlerID)
{
    if (false == mStopDispatcher)
    {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

bool HsmEventDispatcherArduino::enqueueEvent(const HandlerID_t handlerID, const EventID_t event)
{
    __HSM_TRACE_CALL_DEBUG__();
    bool wasAdded = false;

    noInterrupts();// disable interrupts

    if (mEnqueuedEvents.size() < mEnqueuedEvents.capacity())
    {
        EnqueuedEventInfo newEvent;

        newEvent.handlerID = handlerID;
        newEvent.eventID = event;
        mEnqueuedEvents.push_back(newEvent);
        wasAdded = true;
    }

    interrupts();// enable interrupts

    return wasAdded;
}

void HsmEventDispatcherArduino::dispatchEvents()
{
    if (false == mStopDispatcher)
    {
        handleEnqueuedEvents();
        handleTimers();
        HsmEventDispatcherBase::dispatchPendingEvents();
    }
}

void HsmEventDispatcherArduino::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot)
{
    __HSM_TRACE_CALL_ARGS__("timerID=%d, intervalMs=%d, isSingleShot=%d",
                            SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));
    auto it = mRunningTimers.end();

    {
        // CriticalSection lck;
        it = mRunningTimers.find(timerID);
    }

    // new timer
    if (mRunningTimers.end() == it)
    {
        RunningTimerInfo newTimer;

        newTimer.startedAt = millis();
        newTimer.elapseAfter = newTimer.startedAt + intervalMs;

        {
            // CriticalSection lck;
            mRunningTimers.emplace(timerID, newTimer);
        }
    }
    else // restart timer
    {
        it->second.startedAt = millis();
        it->second.elapseAfter = it->second.startedAt + intervalMs;
    }
}

void HsmEventDispatcherArduino::stopTimerImpl(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_ARGS__("timerID=%d", SC2INT(timerID));

    {
        // CriticalSection lck;
        mRunningTimers.erase(timerID);
    }
}

void HsmEventDispatcherArduino::handleEnqueuedEvents()
{
    if (mEnqueuedEvents.size() > 0)
    {
        HandlerID_t prevHandlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        EnqueuedEventHandlerFunc_t callback;
        std::vector<EnqueuedEventInfo> currentEvents;

        noInterrupts();// disable interrupts
        currentEvents = mEnqueuedEvents;
        mEnqueuedEvents.clear();
        interrupts();// enable interrupts

        // need to traverse events in reverce order 
        for (auto it = currentEvents.rbegin(); it != currentEvents.rend(); ++it)
        {
            if (prevHandlerID != it->handlerID)
            {
                callback = getEnqueuedEventHandlerFunc(it->handlerID);
                prevHandlerID = it->handlerID;
            }

            callback(it->eventID);
        }
    }
}

void HsmEventDispatcherArduino::handleTimers()
{
    if (mRunningTimers.size() > 0)
    {
        unsigned long curTime = millis();
        auto it = mRunningTimers.begin();

        while (it != mRunningTimers.end())
        {
            if (curTime >= it->second.elapseAfter)
            {
                if (true == handleTimerEvent(it->first))
                {
                    // fast way to check timer interval without searching the map
                    it->second.elapseAfter = it->second.elapseAfter - it->second.startedAt;
                    it->second.startedAt = millis();
                    it->second.elapseAfter += it->second.startedAt;
                    ++it;
                } else {
                    it = mRunningTimers.erase(it);
                }
            }
            else
            {
                ++it;
            }
        }
    }
}

}