// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSM_HPP
#define HSMCPP_HSM_HPP

#include <memory>

#include "HsmTypes.hpp"
#include "variant.hpp"

namespace hsmcpp {

class IHsmEventDispatcher;

class HierarchicalStateMachine {
public:
    enum class HistoryType { SHALLOW, DEEP };

    enum class TransitionType { INTERNAL_TRANSITION, EXTERNAL_TRANSITION };

    enum class StateActionTrigger { ON_STATE_ENTRY, ON_STATE_EXIT };

    enum class StateAction {
        START_TIMER,    // ARGS: TimerID_t timerID, int32_t intervalMs, bool singleshot
        STOP_TIMER,     // ARGS: TimerID_t timerID
        RESTART_TIMER,  // ARGS: TimerID_t timerID
        TRANSITION,     // ARGS: EventID_t eventID
    };

public:
    explicit HierarchicalStateMachine(const StateID_t initialState);

    // Uses unregisterEventHandler from Dispatcher. Usually HSM has to be destroyed from the same thread it was created.
    virtual ~HierarchicalStateMachine();

    // changes initial state of HSM
    //
    // NOTE: can be called only before initialize()
    void setInitialState(const StateID_t initialState);

    // Uses registerEventHandler from Dispatcher. Usually must be called from the same thread where dispatcher was created.
    //
    // NOTE: after calling this function HSM becomes operable. So HSM structure must be registered BEFORE calling it.
    //       changing structure after this call can result in undefined behaviour and is not advised.
    virtual bool initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher);

    bool isInitialized() const;

    // Releases dispatcher and resets all internal resources. HSM cant be reused after calling this API.
    // Must be called on the same thread as initialize()
    //
    // NOTE: Usually you dont need to call this function directly. The only scenario when it's needed is
    //       for multithreaded environment where it's impossible to delete HSM on the same thread where it was initialized.
    //       Then you must call release() on the Dispatcher's thread before deleting HSM instance on another thread.
    void release();

    void registerFailedTransitionCallback(const HsmTransitionFailedCallback_t& onFailedTransition);

    template <class HsmHandlerClass>
    void registerFailedTransitionCallback(HsmHandlerClass* handler,
                                          HsmTransitionFailedCallbackPtr_t(HsmHandlerClass, onFailedTransition));

    // If state has substates its callbacks will be ignored
    template <class HsmHandlerClass>
    void registerState(const StateID_t state,
                       HsmHandlerClass* handler = nullptr,
                       HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged) = nullptr,
                       HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering) = nullptr,
                       HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting) = nullptr);

    void registerState(const StateID_t state,
                       HsmStateChangedCallback_t onStateChanged = nullptr,
                       HsmStateEnterCallback_t onEntering = nullptr,
                       HsmStateExitCallback_t onExiting = nullptr);

    template <class HsmHandlerClass>
    void registerFinalState(const StateID_t state,
                            const EventID_t event = INVALID_HSM_EVENT_ID,
                            HsmHandlerClass* handler = nullptr,
                            HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged) = nullptr,
                            HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering) = nullptr,
                            HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting) = nullptr);

    void registerFinalState(const StateID_t state,
                            const EventID_t event = INVALID_HSM_EVENT_ID,
                            HsmStateChangedCallback_t onStateChanged = nullptr,
                            HsmStateEnterCallback_t onEntering = nullptr,
                            HsmStateExitCallback_t onExiting = nullptr);

    // TODO: check structure and return FALSE?
    template <class HsmHandlerClass>
    void registerHistory(const StateID_t parent,
                         const StateID_t historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const StateID_t defaultTarget = INVALID_HSM_STATE_ID,
                         HsmHandlerClass* handler = nullptr,
                         HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr);

    void registerHistory(const StateID_t parent,
                         const StateID_t historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const StateID_t defaultTarget = INVALID_HSM_STATE_ID,
                         HsmTransitionCallback_t transitionCallback = nullptr);

    bool registerSubstate(const StateID_t parent, const StateID_t substate);
    template <class HsmHandlerClass>
    bool registerSubstateEntryPoint(const StateID_t parent,
                                    const StateID_t substate,
                                    const EventID_t onEvent = INVALID_HSM_EVENT_ID,
                                    HsmHandlerClass* handler = nullptr,
                                    HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                                    const bool expectedConditionValue = true);
    bool registerSubstateEntryPoint(const StateID_t parent,
                                    const StateID_t substate,
                                    const EventID_t onEvent = INVALID_HSM_EVENT_ID,
                                    const HsmTransitionConditionCallback_t& conditionCallback = nullptr,
                                    const bool expectedConditionValue = true);

    void registerTimer(const TimerID_t timerID, const EventID_t event);

    // TODO: add support for transition actions
    template <typename... Args>
    bool registerStateAction(const StateID_t state,
                             const StateActionTrigger actionTrigger,
                             const StateAction action,
                             Args... args);

    template <class HsmHandlerClass>
    void registerTransition(const StateID_t from,
                            const StateID_t to,
                            const EventID_t onEvent,
                            HsmHandlerClass* handler = nullptr,
                            HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                            HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                            const bool expectedConditionValue = true);

    void registerTransition(const StateID_t from,
                            const StateID_t to,
                            const EventID_t onEvent,
                            HsmTransitionCallback_t transitionCallback = nullptr,
                            HsmTransitionConditionCallback_t conditionCallback = nullptr,
                            const bool expectedConditionValue = true);

    template <class HsmHandlerClass>
    void registerSelfTransition(const StateID_t state,
                                const EventID_t onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmHandlerClass* handler = nullptr,
                                HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                                HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                                const bool expectedConditionValue = true);

    void registerSelfTransition(const StateID_t state,
                                const EventID_t onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmTransitionCallback_t transitionCallback = nullptr,
                                HsmTransitionConditionCallback_t conditionCallback = nullptr,
                                const bool expectedConditionValue = true);

    // If HSM doesnt contain any parallel states returns current active state.
    // Otherwise returns most recently activated state
    StateID_t getLastActiveState() const;
    const std::list<StateID_t>& getActiveStates() const;
    bool isStateActive(const StateID_t state) const;

    // extended version of transition function with all possible arguments
    // use HSM_WAIT_INDEFINITELY for timeoutMs to ignore timeout
    template <typename... Args>
    bool transitionEx(const EventID_t event, const bool clearQueue, const bool sync, const int timeoutMs, Args... args);

    // basic async transition
    template <typename... Args>
    void transition(const EventID_t event, Args... args);

    // same as regular functions, but uses VariantVector_t for arguments
    void transitionWithArgsArray(const EventID_t event, const VariantVector_t& args);
    bool transitionExWithArgsArray(const EventID_t event,
                                   const bool clearQueue,
                                   const bool sync,
                                   const int timeoutMs,
                                   const VariantVector_t& args);

    // sync transition
    template <typename... Args>
    bool transitionSync(const EventID_t event, const int timeoutMs, Args... args);

    // async transition which clears events queue before adding requested event
    template <typename... Args>
    void transitionWithQueueClear(const EventID_t event, Args... args);

    /**
     * @brief Interrupt/signal safe version of transition
     * @details This is a simplified version of transition that can be safely used from an interrupt/signal. There are no
     *          restrictions to use other transition APIs inside an interrupt, but all of them use dynamic heap memory
     *          allocation (which can cause heap corruption on some platfroms).
     *          This version of the transition relies on dispatcher implementation and might not be available
     *          everywhere (please check dispatcher description). It also might fail if internal dispatcher events queue
     *          will get full.
     *
     * @param event hsm event
     *
     * @return true - event was added to queue
     * @return false - failed to add event to queue because it's not supported by dispatcher or queue limit was reached
     *
     * @threadsafe
     * @interruptsafe
     */
    bool transitionInterruptSafe(const EventID_t event);

    template <typename... Args>
    bool isTransitionPossible(const EventID_t event, Args... args);

    /**
     * Start a new timer. If timer with this ID is already running it will be restarted with new settings.
     *
     * @param timerID       unique timer id
     * @param intervalMs    timer interval in milliseconds
     * @param isSingleShot  true - timer will run only once and then will stop
     *                      false - timer will keep running until stopTimer() is called or dispatcher is destroyed
     */
    void startTimer(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot);

    /**
     * Restarts running timer with the same arguments which were provided to startTimer().
     * Does nothing if timer is not running.
     *
     * @param timerID       id of running timer
     */
    void restartTimer(const TimerID_t timerID);

    /**
     * Restarts running timer with the same arguments which were provided to startTimer()
     * Does nothing if timer is not running.
     *
     * @param timerID       id of running timer
     */
    void stopTimer(const TimerID_t timerID);

    // By default log will be written to ./dump.hsmlog file.
    // This location can be overwritten by setting ENV_DUMPPATH environment variable with desired path.
    bool enableHsmDebugging();
    bool enableHsmDebugging(const std::string& dumpPath);
    void disableHsmDebugging();

protected:
    // NOTE: Clients must implement this method for debugging to work. Names should match with the names in scxml file
    virtual std::string getStateName(const StateID_t state) const;
    virtual std::string getEventName(const EventID_t event) const;

private:
    template <typename... Args>
    void makeVariantList(VariantVector_t& vList, Args&&... args);

    bool registerStateActionImpl(const StateID_t state,
                                 const StateActionTrigger actionTrigger,
                                 const StateAction action,
                                 const VariantVector_t& args);
    bool isTransitionPossibleImpl(const EventID_t event, const VariantVector_t& args);

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

// =================================================================================================================
// Template Functions
// =================================================================================================================

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerFailedTransitionCallback(HsmHandlerClass* handler,
                                                                HsmTransitionFailedCallbackPtr_t(HsmHandlerClass,
                                                                                                 onFailedTransition)) {
    registerFailedTransitionCallback(std::bind(onFailedTransition, handler, std::placeholders::_1, std::placeholders::_2));
}

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerState(const StateID_t state,
                                             HsmHandlerClass* handler,
                                             HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged),
                                             HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering),
                                             HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting)) {
    HsmStateChangedCallback_t funcStateChanged;
    HsmStateEnterCallback_t funcEntering;
    HsmStateExitCallback_t funcExiting;

    if (nullptr != handler) {
        if (nullptr != onStateChanged) {
            funcStateChanged = std::bind(onStateChanged, handler, std::placeholders::_1);
        }

        if (nullptr != onEntering) {
            funcEntering = std::bind(onEntering, handler, std::placeholders::_1);
        }

        if (nullptr != onExiting) {
            funcExiting = std::bind(onExiting, handler);
        }
    }

    registerState(state, funcStateChanged, funcEntering, funcExiting);
}

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerFinalState(const StateID_t state,
                                                  const EventID_t event,
                                                  HsmHandlerClass* handler,
                                                  HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged),
                                                  HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering),
                                                  HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting)) {
    HsmStateChangedCallback_t funcStateChanged;
    HsmStateEnterCallback_t funcEntering;
    HsmStateExitCallback_t funcExiting;

    if (nullptr != handler) {
        if (nullptr != onStateChanged) {
            funcStateChanged = std::bind(onStateChanged, handler, std::placeholders::_1);
        }

        if (nullptr != onEntering) {
            funcEntering = std::bind(onEntering, handler, std::placeholders::_1);
        }

        if (nullptr != onExiting) {
            funcExiting = std::bind(onExiting, handler);
        }
    }

    registerFinalState(state, event, funcStateChanged, funcEntering, funcExiting);
}

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerHistory(const StateID_t parent,
                                               const StateID_t historyState,
                                               const HistoryType type,
                                               const StateID_t defaultTarget,
                                               HsmHandlerClass* handler,
                                               HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback)) {
    HsmTransitionCallback_t funcTransitionCallback;

    if (nullptr != handler) {
        if (nullptr != transitionCallback) {
            funcTransitionCallback = std::bind(transitionCallback, handler, std::placeholders::_1);
        }
    }

    registerHistory(parent, historyState, type, defaultTarget, funcTransitionCallback);
}

template <class HsmHandlerClass>
bool HierarchicalStateMachine::registerSubstateEntryPoint(const StateID_t parent,
                                                          const StateID_t substate,
                                                          const EventID_t onEvent,
                                                          HsmHandlerClass* handler,
                                                          HsmTransitionConditionCallbackPtr_t(HsmHandlerClass,
                                                                                              conditionCallback),
                                                          const bool expectedConditionValue) {
    HsmTransitionConditionCallback_t condition;

    if ((nullptr != handler) && (nullptr != conditionCallback)) {
        condition = std::bind(conditionCallback, handler, std::placeholders::_1);
    }

    return registerSubstateEntryPoint(parent, substate, onEvent, condition, expectedConditionValue);
}

template <typename... Args>
bool HierarchicalStateMachine::registerStateAction(const StateID_t state,
                                                   const StateActionTrigger actionTrigger,
                                                   const StateAction action,
                                                   Args... args) {
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, args...);
    return registerStateActionImpl(state, actionTrigger, action, eventArgs);
}

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerTransition(const StateID_t from,
                                                  const StateID_t to,
                                                  const EventID_t onEvent,
                                                  HsmHandlerClass* handler,
                                                  HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback),
                                                  HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback),
                                                  const bool expectedConditionValue) {
    HsmTransitionCallback_t funcTransitionCallback;
    HsmTransitionConditionCallback_t funcConditionCallback;

    if (nullptr != handler) {
        if (nullptr != transitionCallback) {
            funcTransitionCallback = std::bind(transitionCallback, handler, std::placeholders::_1);
        }

        if (nullptr != conditionCallback) {
            funcConditionCallback = std::bind(conditionCallback, handler, std::placeholders::_1);
        }
    }

    registerTransition(from, to, onEvent, funcTransitionCallback, funcConditionCallback, expectedConditionValue);
}

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerSelfTransition(const StateID_t state,
                                                      const EventID_t onEvent,
                                                      const TransitionType type,
                                                      HsmHandlerClass* handler,
                                                      HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback),
                                                      HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback),
                                                      const bool expectedConditionValue) {
    HsmTransitionCallback_t funcTransitionCallback;
    HsmTransitionConditionCallback_t funcConditionCallback;

    if (nullptr != handler) {
        if (nullptr != transitionCallback) {
            funcTransitionCallback = std::bind(transitionCallback, handler, std::placeholders::_1);
        }

        if (nullptr != conditionCallback) {
            funcConditionCallback = std::bind(conditionCallback, handler, std::placeholders::_1);
        }
    }

    registerSelfTransition(state, onEvent, type, funcTransitionCallback, funcConditionCallback, expectedConditionValue);
}

template <typename... Args>
void HierarchicalStateMachine::transition(const EventID_t event, Args... args) {
    (void)transitionEx(event, false, false, 0, args...);
}

template <typename... Args>
bool HierarchicalStateMachine::transitionEx(const EventID_t event,
                                            const bool clearQueue,
                                            const bool sync,
                                            const int timeoutMs,
                                            Args... args) {
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, args...);
    return transitionExWithArgsArray(event, clearQueue, sync, timeoutMs, eventArgs);
}

template <typename... Args>
bool HierarchicalStateMachine::transitionSync(const EventID_t event, const int timeoutMs, Args... args) {
    return transitionEx(event, false, true, timeoutMs, args...);
}

template <typename... Args>
void HierarchicalStateMachine::transitionWithQueueClear(const EventID_t event, Args... args) {
    // NOTE: async transitions always return true
    (void)transitionEx(event, true, false, 0, args...);
}

template <typename... Args>
bool HierarchicalStateMachine::isTransitionPossible(const EventID_t event, Args... args) {
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, args...);

    return isTransitionPossibleImpl(event, eventArgs);
}

template <typename... Args>
void HierarchicalStateMachine::makeVariantList(VariantVector_t& vList, Args&&... args) {
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
    (void)make_variant;
}

}  // namespace hsmcpp

#endif  // HSMCPP_HSM_HPP
