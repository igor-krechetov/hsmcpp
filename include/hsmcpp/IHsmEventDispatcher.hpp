// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_IHSMEVENTDISPATCHER_HPP__
#define __HSMCPP_IHSMEVENTDISPATCHER_HPP__

#include <functional>

namespace hsmcpp
{
#define INVALID_HSM_DISPATCHER_HANDLER_ID          (0)

using HandlerID_t = int;
using TimerID_t = int;

using EventHandlerFunc_t = std::function<void(void)>;
using TimerHandlerFunc_t = std::function<void(const int)>;

class IHsmEventDispatcher
{
public:
    virtual ~IHsmEventDispatcher() {}

    /**
     * Used to start event dispatching. Implementation is optional and depends
     * on individual dispatcher. But it should be non blocking.
     * Returns TRUE if dispatching was successfully started or if it is already running (calling multiple times will have no effect)
     */
    virtual bool start() = 0;

    /**
     * As a general rule registerEventHandler and unregisterEventHandler are
     * not expected to be thread-safe and should to be used from the same thread.
     * But this depends on specific Dispatcher implementation.
     */

    /**
     * returns ID that should be used to unregister event handler
     */
    virtual HandlerID_t registerEventHandler(const EventHandlerFunc_t& handler) = 0;

    virtual void unregisterEventHandler(const HandlerID_t handlerID) = 0;

    /**
     * dispatcher must guarantee that emit call is thread-safe
     */
    virtual void emitEvent() = 0;

    /**
     * Used by HSM to receive notifications about timer events.
     *
     * @param handler       handler function
     *
     * @return              handlerID. This value must be used when starting a new timer
     */
    virtual HandlerID_t registerTimerHandler(const TimerHandlerFunc_t& handler) = 0;

    /**
     * Unregister timer handler. This will automatically stop all timers registered with this handler.
     */
    virtual void unregisterTimerHandler(const HandlerID_t handlerID) = 0;

    /**
     * Start a new timer. If timer with this ID is already running it will be restarted with new settings.
     *
     * @param timerID       unique timer id
     * @param intervalMs    timer interval in milliseconds
     * @param isSingleShot  true - timer will run only once and then will stop
     *                      false - timer will keep running until stopTimer() is called or dispatcher is destroyed
     */
    virtual void startTimer(const HandlerID_t handlerID, const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) = 0;

    /**
     * Restarts running timer with the same arguments which were provided to startTimer().
     * Does nothing if timer is not running.
     *
     * @param timerID       id of running timer
     */
    virtual void restartTimer(const TimerID_t timerID) = 0;

    /**
     * Restarts running timer with the same arguments which were provided to startTimer()
     * Does nothing if timer is not running.
     *
     * @param timerID       id of running timer
     */
    virtual void stopTimer(const TimerID_t timerID) = 0;

    /**
     * Check if timer is running.
     *
     * @param timerID       id of running timer
     *
     * @return              true - timer is running
     *                      false - timer is not running
     */
    virtual bool isTimerRunning(const TimerID_t timerID) = 0;
};

}// namespace hsmcpp

#endif // __HSMCPP_IHSMEVENTDISPATCHER_HPP__
