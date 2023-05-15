// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_IHSMEVENTDISPATCHER_HPP
#define HSMCPP_IHSMEVENTDISPATCHER_HPP

#include "HsmTypes.hpp"

namespace hsmcpp {
/**
 * @brief Event handler callback.
 * @details When using class members, developers are responsible to track corresponding object's lifetime. Clients can notify
 * dispatcher that callback became invalid by returning FALSE.
 * @retval true handler's callback can be used further
 * @retval false handler's callback is invalid and should not be reused
 */
using EventHandlerFunc_t = std::function<bool(void)>;
/**
 * @brief Timer event handler callback.
 * @details When using class members, developers are responsible to track corresponding object's lifetime. Clients can notify
 * dispatcher that callback became invalid by returning FALSE.
 *
 * @param timerID id of the timer event
 * @retval true handler's callback can be used further
 * @retval false handler's callback is invalid and should not be reused
 */
using TimerHandlerFunc_t = std::function<bool(const TimerID_t)>;
/**
 * @brief Handler callback for enqueued events.
 * @details When using class members, developers are responsible to track corresponding object's lifetime. Clients can notify
 * dispatcher that callback became invalid by returning FALSE.
 *
 * @param eventID id of the event
 * @retval true handler's callback can be used further
 * @retval false handler's callback is invalid and should not be reused
 */
using EnqueuedEventHandlerFunc_t = std::function<bool(const EventID_t)>;

/**
 * @brief Handler callback for enqueued actions.
 * @details See IHsmEventDispatcher::enqueueAction() for details.
 */
using ActionHandlerFunc_t = std::function<void()>;

/**
 * @brief IHsmEventDispatcher provides an interface for events dispatcher implementations.
 * @details The IHsmEventDispatcher class defines the standard dispatcher interface that HSM uses to process internal events. It
 * is not supposed to be instantiated directly. Instead, you should subclass it to create platform or framework specific
 * dispatchers.
 * When subclassing IHsmEventDispatcher, at the very least you must implement:
 *  \li registerEventHandler()
 *  \li unregisterEventHandler()
 *  \li emitEvent()
 *
 * For other API you can make empty implementation. This will be sufficient for basic HierarchicalStateMachine functionality.
 * For timers support following API must be implemented:
 *  \li registerTimerHandler()
 *  \li unregisterTimerHandler()
 *  \li startTimer()
 *  \li restartTimer()
 *  \li stopTimer()
 *  \li isTimerRunning()
 *
 * For interrupt-safe transitions you need to implement:
 *  \li registerEnqueuedEventHandler()
 *  \li unregisterEnqueuedEventHandler()
 *  \li enqueueEvent()
 */
class IHsmEventDispatcher {
public:
    /**
     * @brief Destructor.
     *
     * @warning Make sure to release/delete all HSM instances which are using this dispatcher before deleting it. Failing to do
     * so will result in undefined behavior and (usually) a crash.
     */
    virtual ~IHsmEventDispatcher() = default;

    /**
     * @brief Start events dispatching.
     * @details Is called by HierarchicalStateMachine::initialize(). Implementation of this method is optional and depends on
     * individual dispatcher design. Calling this function multiple times should have no effect. In case there is no special
     * start up logic dispatcher implementation must return true.
     *
     * @remark Implementation should be non blocking and doesn't have to be threadsafe.
     *
     * @retval true dispatching was successfully started or it is already running
     * @retval false failed to start events dispatching
     */
    virtual bool start() = 0;

    /**
     * @brief Stop dispatching events.
     * @details Future calls to dispatchEvents() will have no effect.
     * @remark Operation is performed asynchronously. It does not interrupt currently handled event, but will cancel all other
     * pending events.
     */
    virtual void stop() = 0;

    /**
     * @brief Register a new event handler.
     * @details Dispatcher must support registering multiple handlers. Order in which these handlers are triggered is not
     * important.
     * @remark As a general rule registerEventHandler() and unregisterEventHandler() are not expected to be thread-safe and
     * should be used from the same thread. But this depends on specific Dispatcher implementation.
     *
     * @param handler handler callback
     *
     * @return unique handler ID
     */
    virtual HandlerID_t registerEventHandler(const EventHandlerFunc_t& handler) = 0;

    /**
     * @brief Unregister events handler.
     * @param handlerID handler ID received from registerEventHandler()
     */
    virtual void unregisterEventHandler(const HandlerID_t handlerID) = 0;

    /**
     * @brief Register a new handler for enqueued events.
     * @details Dispatcher must support registering multiple handlers. Order in which these handlers are triggered is not
     * important.
     * @remark As a general rule registerEnqueuedEventHandler() and unregisterEnqueuedEventHandler() are not expected to be
     * thread-safe.
     *
     * @param handler handler callback
     *
     * @return unique handler ID
     */
    virtual HandlerID_t registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler) = 0;

    /**
     * @brief Unregister events handler.
     * @param handlerID handler ID received from registerEnqueuedEventHandler()
     */
    virtual void unregisterEnqueuedEventHandler(const HandlerID_t handlerID) = 0;

    /**
     * @brief Add a new event to the queue for later dispatching.
     * @details Dispatcher should initiate processing of the new event as soon as possible.
     *
     * @param handlerID id of the handler that should process the event
     *
     * @threadsafe{Dispatcher implementation **must guarantee** that this call is thread-safe.}
     */
    virtual void emitEvent(const HandlerID_t handlerID) = 0;

    /**
     * @brief Add a new event to the queue for later dispatching.
     * @details Behaves same way as emitEvent(), but is intended to be used only from inside signals/interrupts.
     * Unlike emitEvent(), there is usually a limit of how many events can be added to the queue
     * at the same time (depends on individual implementation).
     *
     * @warning Implementation of this method **SHOULD NOT** use dynamic memory allocations.
     *
     * @param handlerID     id of the handler that should be called
     * @param event         id of the hsm event
     *
     * @retval true event was successfully added
     * @retval false failed to add event because internal queue is full
     *
     * @concurrencysafe{Dispatcher implementation **must guarantee** that this call is thread-safe and signals/interrupts safe.}
     */
    virtual bool enqueueEvent(const HandlerID_t handlerID, const EventID_t event) = 0;

    /**
     * @brief Enqueue action to be executed on dispatcher's thread.
     * @details Enqueued actions have highest priority compared to events and will be executed as soon as possible. If there are
     * any events being processed then dispatcher will first finish their execution.
     *
     * @param actionCallback  functor to be called by dispatcher
     */
    virtual void enqueueAction(ActionHandlerFunc_t actionCallback) = 0;

    /**
     * @brief Register a new handler for timers.
     * @details Dispatcher must support registering multiple handlers. Order in which these handlers are triggered is not
     * important. Used by HierarchicalStateMachine to receive notifications about timer events.
     *
     * @param handler       handler callback
     *
     * @return              unique handler ID which must be used when starting a new timer with startTimer()
     */
    virtual HandlerID_t registerTimerHandler(const TimerHandlerFunc_t& handler) = 0;

    /**
     * @brief Unregister timer handler.
     * @details This will automatically stop all timers registered with this handler.
     *
     * @param handlerID handler ID received from registerTimerHandler()
     */
    virtual void unregisterTimerHandler(const HandlerID_t handlerID) = 0;

    /**
     * @brief Start a timer.
     * @details If timer with this ID is already running it will be restarted with new settings.
     *
     * @param handlerID     handler id for wich to start the timer (returned from registerTimerHandler())
     * @param timerID       unique timer id
     * @param intervalMs    timer interval in milliseconds
     * @param isSingleShot  true - timer will run only once and then will stop
     *                      false - timer will keep running until stopTimer() is called or dispatcher is destroyed
     *
     * @threadsafe{Dispatcher implementation **must guarantee** that this call is thread-safe.}
     */
    virtual void startTimer(const HandlerID_t handlerID,
                            const TimerID_t timerID,
                            const unsigned int intervalMs,
                            const bool isSingleShot) = 0;

    /**
     * @brief Restart running or expired timer.
     * @details Timer is restarted with the same arguments which were provided to startTimer(). Only currently running or
     * expired timers (with isSingleShot set to true) will be restarted. Has no effect if called for a timer which was not
     * started.
     *
     * @param timerID       id of running timer
     *
     * @threadsafe{Dispatcher implementation **must guarantee** that this call is thread-safe.}
     */
    virtual void restartTimer(const TimerID_t timerID) = 0;

    /**
     * @brief Stop active timer.
     * @details Function stops an active timer without triggering any notifications and unregisters it. Further calls to
     * restartTimer() will have no effects untill it's started again with startTimer().
     *
     * @remark For expired timers (which have isSingleShot property set to true), funtion simply unregisters them.
     *
     * @param timerID id of running or expired timer
     *
     * @threadsafe{Dispatcher implementation **must guarantee** that this call is thread-safe.}
     */
    virtual void stopTimer(const TimerID_t timerID) = 0;

    /**
     * @brief Check if timer is currently running.
     *
     * @param timerID id of the timer to check
     *
     * @retval true timer is running
     * @retval false timer is not running
     *
     * @threadsafe{Dispatcher implementation **must guarantee** that this call is thread-safe.}
     */
    virtual bool isTimerRunning(const TimerID_t timerID) = 0;
};

}  // namespace hsmcpp

#endif  // HSMCPP_IHSMEVENTDISPATCHER_HPP
