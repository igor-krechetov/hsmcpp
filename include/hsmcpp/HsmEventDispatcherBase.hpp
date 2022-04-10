// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__

#include <map>
#include <list>
#include "os/Mutex.hpp"
#include "IHsmEventDispatcher.hpp"

namespace hsmcpp
{

class HsmEventDispatcherBase: public IHsmEventDispatcher
{
protected:
    struct TimerInfo
    {
        HandlerID_t handlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        unsigned int intervalMs = 0;
        bool isSingleShot = false;
    };

public:
    virtual ~HsmEventDispatcherBase();

    virtual HandlerID_t registerEventHandler(const EventHandlerFunc_t& handler) override;
    virtual void unregisterEventHandler(const HandlerID_t handlerID) override;
    virtual void emitEvent(const HandlerID_t handlerID) = 0;

    virtual HandlerID_t registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler) override;
    virtual void unregisterEnqueuedEventHandler(const HandlerID_t handlerID) override;
    virtual bool enqueueEvent(const HandlerID_t handlerID, const EventID_t event) override;

    virtual HandlerID_t registerTimerHandler(const TimerHandlerFunc_t& handler) override;
    virtual void unregisterTimerHandler(const HandlerID_t handlerID) override;

    virtual void startTimer(const HandlerID_t handlerID,
                            const TimerID_t timerID,
                            const unsigned int intervalMs,
                            const bool isSingleShot) override;
    virtual void restartTimer(const TimerID_t timerID) override;
    virtual void stopTimer(const TimerID_t timerID) override;
    virtual bool isTimerRunning(const TimerID_t timerID) override;

protected:
    virtual HandlerID_t getNextHandlerID();

    void unregisterAllEventHandlers();

    EnqueuedEventHandlerFunc_t getEnqueuedEventHandlerFunc(const HandlerID_t handlerID) const;

    TimerInfo getTimerInfo(const TimerID_t timerID) const;
    TimerHandlerFunc_t getTimerHandlerFunc(const HandlerID_t handlerID) const;

    virtual void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot);
    virtual void stopTimerImpl(const TimerID_t timerID);

    /**
     * Handles common logic for handling timer events
     * 
     * @param timerID     id of the expired timer
     *
     * @return TRUE if timer should be restarted, FALSE if timer can be deleted
     */
    bool handleTimerEvent(const TimerID_t timerID);

    void dispatchPendingEvents();
    void dispatchPendingEvents(const std::list<HandlerID_t>& events);

protected:
    HandlerID_t mNextHandlerId = 1;
    std::map<TimerID_t, TimerInfo> mActiveTimers;
    std::map<HandlerID_t, EventHandlerFunc_t> mEventHandlers;
    std::map<HandlerID_t, EnqueuedEventHandlerFunc_t> mEnqueuedEventHandlers;
    std::map<HandlerID_t, TimerHandlerFunc_t> mTimerHandlers;
    std::list<HandlerID_t> mPendingEvents;
    Mutex mEmitSync;
    Mutex mHandlersSync;
};

}// namespace hsmcpp
#endif // __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__
