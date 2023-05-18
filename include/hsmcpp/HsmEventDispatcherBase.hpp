// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERBASE_HPP
#define HSMCPP_HSMEVENTDISPATCHERBASE_HPP

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "IHsmEventDispatcher.hpp"
#include "os/Mutex.hpp"

/**
 * @details Default queue size used for enqueued events.
 */
#define DISPATCHER_DEFAULT_EVENTS_CACHESIZE (10)

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherBase provides common implementation for all dispatchers.
 * @details Class contains all platform/framework independent logic for dispatchers. Even though it's not mandatory to use
 * HsmEventDispatcherBase when implementing your own dispatcher, it's highly probable that this class will be a better choice
 * than directly subclassing from IHsmEventDispatcher.
 */
class HsmEventDispatcherBase : public IHsmEventDispatcher {
protected:
    struct TimerInfo {
        HandlerID_t handlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        unsigned int intervalMs = 0;
        bool isSingleShot = false;
    };

    struct EnqueuedEventInfo {
        HandlerID_t handlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        EventID_t eventID = INVALID_HSM_EVENT_ID;

        EnqueuedEventInfo(const HandlerID_t newHandlerID, const EventID_t newEventID);
    };

public:
    /**
     * @brief See IHsmEventDispatcher::stop()
     * @threadsafe{ }
     */
    void stop() override;

    /**
     * @brief See IHsmEventDispatcher::registerEventHandler()
     * @threadsafe{ }
     */
    HandlerID_t registerEventHandler(const EventHandlerFunc_t& handler) override;

    /**
     * @brief See IHsmEventDispatcher::unregisterEventHandler()
     * @threadsafe{ }
     */
    void unregisterEventHandler(const HandlerID_t handlerID) override;

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{ }
     */
    void emitEvent(const HandlerID_t handlerID) override = 0;

    /**
     * @brief See IHsmEventDispatcher::enqueueEvent()
     * @concurrencysafe{ }
     */
    bool enqueueEvent(const HandlerID_t handlerID, const EventID_t event) override;

    /**
     * @brief See IHsmEventDispatcher::enqueueAction()
     * @threadsafe{ }
     */
    void enqueueAction(ActionHandlerFunc_t actionCallback) override;

    /**
     * @brief See IHsmEventDispatcher::registerEnqueuedEventHandler()
     * @threadsafe{ }
     */
    HandlerID_t registerEnqueuedEventHandler(const EnqueuedEventHandlerFunc_t& handler) override;

    /**
     * @brief See IHsmEventDispatcher::unregisterEnqueuedEventHandler()
     * @threadsafe{ }
     */
    void unregisterEnqueuedEventHandler(const HandlerID_t handlerID) override;

    /**
     * @brief See IHsmEventDispatcher::registerTimerHandler()
     * @threadsafe{ }
     */
    HandlerID_t registerTimerHandler(const TimerHandlerFunc_t& handler) override;

    /**
     * @brief See IHsmEventDispatcher::unregisterTimerHandler()
     * @threadsafe{ }
     */
    void unregisterTimerHandler(const HandlerID_t handlerID) override;

    /**
     * @brief See IHsmEventDispatcher::startTimer()
     * @threadsafe{ }
     */
    void startTimer(const HandlerID_t handlerID,
                    const TimerID_t timerID,
                    const unsigned int intervalMs,
                    const bool isSingleShot) override;

    /**
     * @brief See IHsmEventDispatcher::restartTimer()
     * @threadsafe{ }
     */
    void restartTimer(const TimerID_t timerID) override;

    /**
     * @brief See IHsmEventDispatcher::stopTimer()
     * @threadsafe{ }
     */
    void stopTimer(const TimerID_t timerID) override;

    /**
     * @brief See IHsmEventDispatcher::isTimerRunning()
     * @threadsafe{ }
     */
    bool isTimerRunning(const TimerID_t timerID) override;

protected:
    /**
     * @brief Default constructor.
     * @param eventsCacheSize size of the queue preallocated for delayed events
     */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    explicit HsmEventDispatcherBase(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

    /**
     * Destructor.
     */
    virtual ~HsmEventDispatcherBase() = default;

    static void handleDelete(HsmEventDispatcherBase* dispatcher);

    /**
     * @brief Schedule instance for deletion.
     * @retval false instance will be deleted later automatically
     * @retval true instance can be deleted now
     *
     * @notthreadsafe{Should not be called multiple times}
     */
    virtual bool deleteSafe() = 0;

    /**
     * @brief Generate new unique ID for handler.
     * @details To avoid unintended errors, even different type of handlers have unique IDs. Current implementation just returns
     * an incremended ID.
     *
     * @return new unique handler ID
     *
     * @notthreadsafe{Used in registerXxxxxHandler() API which are not required to be thread-safe.}
     */
    virtual HandlerID_t getNextHandlerID();

    /**
     * @brief Unregister all event handlers.
     *
     * @threadsafe{ }
     */
    void unregisterAllEventHandlers();

    /**
     * @brief Find enqueued events handler callback.
     *
     * @param handlerID enqueued events handler id
     *
     * @return Enqueued events handler callback or nullptr if handlerID is invalid.
     *
     * @notthreadsafe{Used only in dispatchEnqueuedEvents(), but data race can happen if registerEnqueuedEventHandler() or
     * unregisterEnqueuedEventHandler() are called from a different thread.}
     */
    EnqueuedEventHandlerFunc_t getEnqueuedEventHandlerFunc(const HandlerID_t handlerID) const;

    /**
     * @brief Find timer handler callback.
     *
     * @param handlerID timer handler id
     *
     * @return Timer handler callback or nullptr if handlerID is invalid.
     *
     * @notthreadsafe{Used only in handleTimerEvent(), but data race can happen if registerTimerHandler() or
     * unregisterTimerHandler() are called from a different thread.}
     */
    TimerHandlerFunc_t getTimerHandlerFunc(const HandlerID_t handlerID) const;

    /**
     * @brief Platform specific implementation to start a timer.
     * @details Must be implemented by derived classes. Default implementation does nothing.
     *
     * @param timerID       unique timer id
     * @param intervalMs    timer interval in milliseconds
     * @param isSingleShot  true - timer will run only once and then will stop
     *                      false - timer will keep running until stopTimer() is called or dispatcher is destroyed
     */
    virtual void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot);

    /**
     * @brief Platform specific implementation to stop a timer.
     * @details Must be implemented by derived classes. Default implementation does nothing.
     *
     * @param timerID id of running or expired timer
     */
    virtual void stopTimerImpl(const TimerID_t timerID);

    /**
     * @brief Contains common logic for handling timer event
     *
     * @param timerID id of the expired timer
     *
     * @retval true timer should be restarted
     * @retval false timer can be deleted (usually because it's a singleshot timer)
     */
    bool handleTimerEvent(const TimerID_t timerID);

    /**
     * @brief Wakeup dispatching thread to process pending events.
     * @details Must be implemented by derived classes.
     */
    virtual void notifyDispatcherAboutEvent() = 0;

    /**
     * @brief Dispatch currently enqueued events.
     * @details Should be called by derived classes in dedicated dispatcher thread.
     *
     * @threadsafe{ }
     */
    void dispatchEnqueuedEvents();

    /**
     * @brief Dispatch currently enqueued actions.
     * @details Should be called by derived classes in dedicated dispatcher thread.
     *
     * @threadsafe{ }
     */
    void dispatchPendingActions();

    /**
     * @brief Dispatch all pending events.
     * @details Should be called by derived classes in dedicated dispatcher thread.
     *
     * @threadsafe{ }
     */
    void dispatchPendingEvents();

    /**
     * @brief Dispatch pending events based on provided list.
     *
     * @param events list of events to dispatch
     *
     * @threadsafe{ }
     */
    void dispatchPendingEventsImpl(const std::list<HandlerID_t>& events);

protected:
    HandlerID_t mNextHandlerId = 1;
    std::map<TimerID_t, TimerInfo> mActiveTimers;                              // protected by mHandlersSync
    std::map<HandlerID_t, EventHandlerFunc_t> mEventHandlers;                  // protected by mHandlersSync
    std::map<HandlerID_t, EnqueuedEventHandlerFunc_t> mEnqueuedEventHandlers;  // protected by mHandlersSync
    std::map<HandlerID_t, TimerHandlerFunc_t> mTimerHandlers;                  // protected by mHandlersSync
    std::list<ActionHandlerFunc_t> mPendingActions;                            // protected by mEmitSync
    std::list<HandlerID_t> mPendingEvents;                                     // protected by mEmitSync
    std::vector<EnqueuedEventInfo> mEnqueuedEvents;                            // protected by mEnqueuedEventsSync
    Mutex mEmitSync;
    Mutex mHandlersSync;
    Mutex mEnqueuedEventsSync;
    Mutex mRunningTimersSync;
    bool mStopDispatcher = false;
};

}  // namespace hsmcpp
#endif  // HSMCPP_HSMEVENTDISPATCHERBASE_HPP
