// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_IHSMEVENTDISPATCHER_HPP
#define HSMCPP_IHSMEVENTDISPATCHER_HPP

#include <functional>

namespace hsmcpp
{
#define INVALID_HSM_DISPATCHER_HANDLER_ID           (0)
#define INVALID_HSM_TIMER_ID                        (-1000)


using HandlerID_t = int32_t;
using TimerID_t = int32_t;
using EventID_t = int32_t;

using EventHandlerFunc_t = std::function<void(void)>;
using TimerHandlerFunc_t = std::function<void(const TimerID_t)>;
using EnqueuedEventHandlerFunc_t = std::function<void(const EventID_t)>;

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
     * not expected to be thread-safe and should be used from the same thread.
     * But this depends on specific Dispatcher implementation.
     */

    /**
     * returns ID that should be used to unregister event handler
     */
    virtual HandlerID_t registerEventHandler(const EventHandlerFunc_t& handler) = 0;

    virtual void unregisterEventHandler(const HandlerID_t handlerID) = 0;

    /**
     * TODO
     */
    virtual HandlerID_t registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler) = 0;
    virtual void unregisterEnqueuedEventHandler(const HandlerID_t handlerID) = 0;

    /**
     * Adds new event to the queue. Dispatcher must guarantee that emit call is thread-safe.
     *
     * @param handlerID     id of the handler that should be called
     */
    virtual void emitEvent(const HandlerID_t handlerID) = 0;

    /**
     * Same as emitEvent(), but is intended to be used only from inside interrupts.
     * Unlike emitEvent(), there is a limit of how many events can be added to the queue
     * at the same time.
     *
     * @param handlerID     id of the handler that should be called
     * @param event         id of the hsm event
     *
     * @return              returns true if event was successfully added, or false if events queue is full
     */
    virtual bool enqueueEvent(const HandlerID_t handlerID, const EventID_t event) = 0;

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

#endif // HSMCPP_IHSMEVENTDISPATCHER_HPP
