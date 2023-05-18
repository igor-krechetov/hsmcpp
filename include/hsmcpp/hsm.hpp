// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSM_HPP
#define HSMCPP_HSM_HPP

#include <list>
#include <memory>
#include <string>

#include "HsmTypes.hpp"
#include "variant.hpp"

namespace hsmcpp {

class IHsmEventDispatcher;

/**
 * @brief Implements a Hierarchical State Machine (HSM) for event-driven systems.
 *
 * @details Represents a hierarchical state machine (HSM) with a set of states and state transitions.
 * This class allows for the creation and manipulation of a state machine, where the HSM can be in one
 * of a finite set of states, and can transition between states based on a set of defined rules.
 * The HSM can have multiple levels of nested states and transitions can be defined between any two states,
 * including self-transitions.
 * The class provides public functions to:
 *      \li define state machine structure
 *      \li define transition rules
 *      \li register state and transition callbacks
 *      \li trigger state transitions
 *      \li interact with HSM timers
 */
class HierarchicalStateMachine {
public:
    /**
     * @brief Constructor that sets the initial state of the HSM.
     *
     * @details Initial state can be modified later with setInitialState().
     *
     * @param initialState The initial state of the HSM.
     */
    explicit HierarchicalStateMachine(const StateID_t initialState);

    /**
     * @brief Destructor.

     * @notthreadsafe{Internally uses release().}
     */
    virtual ~HierarchicalStateMachine();

    /**
     * @brief Sets the initial state of the HSM.
     * @remark Has no effect when called after initialize().
     *
     * @param initialState The initial state of the HSM.
     *
     * @concurrencysafe{ }
     */
    void setInitialState(const StateID_t initialState);

    /**
     * @brief Initializes the HSM.
     * @details Registers HSM with provided event dispatcher and transitions state machine into it's initial state. HSM
     * structure must be registered **BEFORE** calling it. Changing structure after this call can result in undefined behavior
     * and is not advised.
     *
     * @remark If initial state has registered callbacks or actions they will be executed synchronously during
     * initialize() call.
     *
     * @warning HSM does not take ownership of the dispatcher. User is responsible for keeping dispatcher instance alive as long
     * as HSM object exists or until call to release().
     *
     * @param dispatcher An event dispatcher that can be used to receive events and dispatch them to the HSM.
     * @return true if initialization succeeds, false otherwise.
     *
     * @notthreadsafe{Internally uses IHsmEventDispatcher::registerEventHandler() and IHsmEventDispatcher::start(). Usually must
     * be called from the same thread where dispatcher was created.}
     */
    virtual bool initialize(const std::weak_ptr<IHsmEventDispatcher>& dispatcher);

    /**
     * @brief Returns dispatcher that was passed to initialize() method.
     *
     * @return dispatcher used by HSM or nullptr (if HSM was not initialized or release() was called).
     *
     * @threadsafe{ }
     */
    std::weak_ptr<IHsmEventDispatcher> dispatcher() const;

    /**
     * @brief Checks initialization status of HSM.
     *
     * @retval true HSM is initialized
     * @retval false HSM is not initialized
     *
     * @concurrencysafe{ }
     */
    bool isInitialized() const;

    /**
     * @brief Releases dispatcher instance and frees any allocated internal resources.
     * @details Internally calls IHsmEventDispatcher::unregisterEventHandler(). Usually, HSM has to be released on the same
     * thread it was initialized.
     *
     * @warning HSM can't be reused after calling this API.
     *
     * @note Usually you don't need to call this function directly. The only scenario when it's needed is for multithreaded
     * environment where it's impossible to delete HSM instance on the same thread where it was initialized. In this case you
     * call release() on the dispatcher's thread before deleting HSM instance on another thread.
     *
     * @notthreadsafe{Usually must be called on the same thread as initialize(). Releases reference to dispatcher. In case
     * HierarchicalStateMachine object is the only one owning IHsmEventDispatcher reference, then you need to check the
     * limitations for deleting dispatcher instance. See the documentation for the used dispatcher.}
     */
    void release();

    /**
     * @brief Registers a callback function to be called when a transition fails.
     * @details Transition failure is usually caused by:
     *      \li no defined transition from the current active state for triggered event
     *      \li false conditions for all matching transitions
     *      \li transition was blocked by exit or enter callback returning false
     *
     * @param onFailedTransition The callback function to be called when transition fails.
     *
     * @concurrencysafe{ }
     */
    void registerFailedTransitionCallback(HsmTransitionFailedCallback_t onFailedTransition);

    /**
     * @brief Registers a class member as a callback function to be called when a transition fails.
     * @copydetails registerFailedTransitionCallback()
     *
     * @param handler Pointer to an object whose class members will be used as callbacks.
     */
    template <class HsmHandlerClass>
    void registerFailedTransitionCallback(HsmHandlerClass* handler,
                                          HsmTransitionFailedCallbackPtr_t(HsmHandlerClass, onFailedTransition));

    /**
     * @brief Registers a new state and optional state callbacks.
     *
     * @param state unique ID of the state to be registered.
     * @param onStateChanged (optional) callback function to be called when the state became active.
     * @param onEntering (optional) callback function to be called when entering the state.
     * @param onExiting (optional) callback function to be called before exiting the state.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    void registerState(const StateID_t state,
                       HsmStateChangedCallback_t onStateChanged = nullptr,
                       HsmStateEnterCallback_t onEntering = nullptr,
                       HsmStateExitCallback_t onExiting = nullptr);

    /**
     * @brief Registers a new state and optional state callbacks (using class members).
     * @copydetails registerState()
     * @tparam handler Pointer to an object whose class members will be used as callbacks.
     *
     * @warning If handler object is destroyed while HSM instance is still running it will result in a crash.
     */
    template <class HsmHandlerClass>
    void registerState(const StateID_t state,
                       HsmHandlerClass* handler = nullptr,
                       HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged) = nullptr,
                       HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering) = nullptr,
                       HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting) = nullptr);

    /**
     * @brief Registers state as final.
     *
     * @details See @rstref{features-substates-final_state} for details.
     *
     * @param state unique ID of the state to be registered as final.
     * @param event (optional) event ID to automatically trigger when entering a final state. If not set (INVALID_HSM_EVENT_ID
     * value is used) then HSM will trigger same event which was used to transition into this final state.
     * @param onStateChanged (optional) callback function to be called when the state became active.
     * @param onEntering (optional) callback function to be called when entering the state.
     * @param onExiting (optional) callback function to be called before exiting the state.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    void registerFinalState(const StateID_t state,
                            const EventID_t event = INVALID_HSM_EVENT_ID,
                            HsmStateChangedCallback_t onStateChanged = nullptr,
                            HsmStateEnterCallback_t onEntering = nullptr,
                            HsmStateExitCallback_t onExiting = nullptr);

    /**
     * @brief Registers a state as final using class members as callbacks
     * @copydetails registerFinalState()
     *
     * @warning If handler object is destroyed while HSM instance is still running it will result in a crash.
     *
     * @param handler Pointer to an object whose class members will be used as callbacks.
     */
    template <class HsmHandlerClass>
    void registerFinalState(const StateID_t state,
                            const EventID_t event = INVALID_HSM_EVENT_ID,
                            HsmHandlerClass* handler = nullptr,
                            HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged) = nullptr,
                            HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering) = nullptr,
                            HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting) = nullptr);

    /**
     * @brief Registers a history state with the state machine.
     *
     * @details See @rstref{features-history} for details.
     *
     * @param parent ID of the parent state.
     * @param historyState ID of the state to be registered as history state.
     * @param type type of history to be used.
     * @param defaultTarget ID of the default target state to be used when transitioning into empty history state.
     * @param transitionCallback transition callback function to be called when the history state is entered.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    // TODO: check structure and return FALSE?
    void registerHistory(const StateID_t parent,
                         const StateID_t historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const StateID_t defaultTarget = INVALID_HSM_STATE_ID,
                         HsmTransitionCallback_t transitionCallback = nullptr);

    /**
     * @brief Registers a history state with the state machine (using class member as a callback)
     * @copydetails registerHistory()
     *
     * @warning If handler object is destroyed while HSM instance is still running it will result in a crash.
     *
     * @param handler Pointer to an object whose class members will be used as callbacks.
     */
    template <class HsmHandlerClass>
    void registerHistory(const StateID_t parent,
                         const StateID_t historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const StateID_t defaultTarget = INVALID_HSM_STATE_ID,
                         HsmHandlerClass* handler = nullptr,
                         HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr);

    /**
     * @brief Registers state as a substate.
     * @details Substate must be first registered with a call to registerState().
     * HSM will make a sanity structure check if HSM_ENABLE_SAFE_STRUCTURE. Following cases are not allowed:
     *      \li parent and substate can't be same
     *      \li substate can't belong to multiple parents
     *      \li circular dependencies are not allowed (A -> B -> A -> ...)
     * See @rstref{features-substates} for details.
     *
     * @param parent ID of the parent state.
     * @param substate ID of the state to be registered as substate
     * @retval true substate was successfully registered
     * @retval false registering substate is not allowed
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    bool registerSubstate(const StateID_t parent, const StateID_t substate);

    /**
     * @brief Registers an entry point for a parent state.
     * @details Entry point state must be first registered with a call to registerState().
     * HSM will make a sanity structure check if HSM_ENABLE_SAFE_STRUCTURE. Following cases are not allowed:
     *      \li parent and entry point can't be same
     *      \li entry point can't belong to multiple parents
     *      \li circular dependencies are not allowed (A -> B -> A -> ...)
     * See @rstref{features-substates-entry_points} for details.
     *
     * @param parent ID of the parent state.
     * @param substate ID of the substate to be registered as an entry point.
     * @param onEvent (optional) ID of the event that should match event that caused activation of the parent state.
     * @param conditionCallback (optional) callback function that will be called to determine if the transition to the
     * entry point is allowed or not.
     * @param expectedConditionValue (optional) expected value from the condition callback to allow transition to the entry
     * state.
     * @retval true substate was successfully registered
     * @retval false registering substate is not allowed
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    bool registerSubstateEntryPoint(const StateID_t parent,
                                    const StateID_t substate,
                                    const EventID_t onEvent = INVALID_HSM_EVENT_ID,
                                    HsmTransitionConditionCallback_t conditionCallback = nullptr,
                                    const bool expectedConditionValue = true);

    /**
     * @brief Registers an entry point for a parent state with class member as a callback.
     * @copydetails registerSubstateEntryPoint()
     *
     * @warning If handler object is destroyed while HSM instance is still running it will result in a crash.
     *
     * @param handler Pointer to an object whose class members will be used as callbacks.
     */
    template <class HsmHandlerClass>
    bool registerSubstateEntryPoint(const StateID_t parent,
                                    const StateID_t substate,
                                    const EventID_t onEvent = INVALID_HSM_EVENT_ID,
                                    HsmHandlerClass* handler = nullptr,
                                    HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                                    const bool expectedConditionValue = true);

    /**
     * @brief Registers a timer to be used inside HSM.
     * @details This function registers new timer and it's event. When the timer expires, the event
     * is sent to the state machine.
     *
     * @param timerID unique ID of the timer.
     * @param event ID of the event to send when timer expires.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    void registerTimer(const TimerID_t timerID, const EventID_t event);

    // TODO: add support for transition actions
    /**
     * @brief Registers a state action with optional arguments.
     * @details The action will be triggered depending on the specified action trigger when the state is activated.
     * See @rstref{features-states-actions} for details.
     *
     * @param state ID of the state for actions are being registered.
     * @param actionTrigger trigger for the state action.
     * @param action action type to register.
     * @param args optional arguments for the state action (see StateAction enum for details).
     * @retval true state action was registered
     * @retval false action registration failed due to invalid arguments
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    template <typename... Args>
    bool registerStateAction(const StateID_t state,
                             const StateActionTrigger actionTrigger,
                             const StateAction action,
                             Args&&... args);

    /**
     * @brief Registers a transition from one state to another.
     * @details The transition will be triggered by the specified event when the **from** state is active.
     * See @rstref{features-transitions} for details.
     *
     * @param from ID of the state to transition from.
     * @param to ID of the state to transition to.
     * @param onEvent ID of the event that triggers the transition.
     * @param transitionCallback (optional) callback function that will be called when transition occurs.
     * @param conditionCallback (optional) callback function that will be called to determine if the transition is allowed.
     * @param expectedConditionValue (optional) expected value from the condition callback function to allow transition.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    void registerTransition(const StateID_t fromState,
                            const StateID_t toState,
                            const EventID_t onEvent,
                            HsmTransitionCallback_t transitionCallback = nullptr,
                            HsmTransitionConditionCallback_t conditionCallback = nullptr,
                            const bool expectedConditionValue = true);

    /**
     * @brief Registers a transition from one state to another.
     * @copydetails registerTransition()
     *
     * @warning If handler object is destroyed while HSM instance is still running it will result in a crash.
     *
     * @param handler Pointer to an object whose class members will be used as callbacks.
     */
    template <class HsmHandlerClass>
    void registerTransition(const StateID_t fromState,
                            const StateID_t toState,
                            const EventID_t onEvent,
                            HsmHandlerClass* handler = nullptr,
                            HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                            HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                            const bool expectedConditionValue = true);

    /**
     * @brief Register a self-transition for a state.
     * @details This function registers a self-transition for a given state. A self-transition is a transition from a state
     * to itself, triggered by a specific event. The transition can be either internal or external. The user can also register a
     * transition callback and a transition condition callback.
     * See @rstref{features-transitions-selftransitions} for details.
     *
     * @param state ID of the state to register self-transition for
     * @param onEvent ID of event that triggers self-transition
     * @param type type of self transition
     * @param transitionCallback A function that is called when the transition occurs (default: nullptr).
     * @param conditionCallback A function that is called to determine if the transition is allowed (default: nullptr).
     * @param expectedConditionValue The expected value returned by the condition callback (default: true).
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    void registerSelfTransition(const StateID_t state,
                                const EventID_t onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmTransitionCallback_t transitionCallback = nullptr,
                                HsmTransitionConditionCallback_t conditionCallback = nullptr,
                                const bool expectedConditionValue = true);

    /**
     * @brief Register a self-transition for a state (using class members as callback).
     * @copydetails registerSelfTransition()
     *
     * @warning If handler object is destroyed while HSM instance is still running it will result in a crash.
     *
     * @param handler Pointer to an object whose class members will be used as callbacks.
     */
    template <class HsmHandlerClass>
    void registerSelfTransition(const StateID_t state,
                                const EventID_t onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmHandlerClass* handler = nullptr,
                                HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                                HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                                const bool expectedConditionValue = true);

    /**
     * @brief Get the ID of the last activated state.
     * @details Returns current active state if HSM doesn't contain any parallel states. Otherwise returns most recently
     * activated state.
     *
     * @return ID of the last active state.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    StateID_t getLastActiveState() const;

    /**
     * @brief Get the list of currently active states.
     *
     * @return list of currently active states.
     */
    const std::list<StateID_t>& getActiveStates() const;

    /**
     * @brief Check if a state is active.
     * @details This function checks if a specific state is currently active in the HSM.
     *
     * @param state ID of the state to check
     * @return True if the state is active, false otherwise.
     */
    bool isStateActive(const StateID_t state) const;

    /**
     * @brief Trigger a transition in the HSM.
     * @details This function sends event to HSM to trigger a potential transition. The transition is executed asynchronously,
     * and any registered callbacks are called in the order they were registered. The user can optionally provide arguments
     * which will be passed to all callbacks triggered by transition.
     *
     * @param event ID of event to send to HSM
     * @param args (optional) arguments to pass to the callbacks
     *
     * @threadsafe{ }
     */
    template <typename... Args>
    void transition(const EventID_t event, Args&&... args);

    /**
     * @brief Trigger a transition in the HSM.
     * @details This is an extended version of transition() function. It also allows to sends an event to HSM to trigger a
     * potential transition, but provides a bit more capabilities.
     *
     * @warning setting sync=true when calling this function from HSM callback will result in blocking HSM events processing and
     * will result in a deadlock if timeoutMs is set to HSM_WAIT_INDEFINITELY.
     *
     * @param event ID of event to send to HSM
     * @param clearQueue indicates whether to clear the pending events queue before adding a new event
     * @param sync indicates whether to wait for the transition to complete before returning. Keep in mind that this **does not
     * cancel** transition if it couldn't finish before timeoutMs. If you need to guarantee that transition was fully processed
     * make sure to set timeoutMs to HSM_WAIT_INDEFINITELY.
     * @param timeoutMs maximum time in milliseconds to wait for the transition to complete if sync is true. Use
     * HSM_WAIT_INDEFINITELY to wait indefinitely.
     * @param args (optional) arguments to pass to the callbacks
     *
     * @return always returns true if sync=false.
     * @retval true (if sync=true) event was accepted and transition successfully finished
     * @retval false (if sync=true) no matching transitions were found, transition was canceled or timeoutMs expired
     *
     * @threadsafe{ }
     */
    template <typename... Args>
    bool transitionEx(const EventID_t event, const bool clearQueue, const bool sync, const int timeoutMs, Args&&... args);

    /**
     * @brief Trigger a transition in the HSM with arguments passed as a vector.
     * @copydetails transition()
     */
    void transitionWithArgsArray(const EventID_t event, VariantVector_t&& args);

    /**
     * @brief Trigger a transition in the HSM with arguments passed as a vector.
     * @copydetails transitionEx()
     */
    bool transitionExWithArgsArray(const EventID_t event,
                                   const bool clearQueue,
                                   const bool sync,
                                   const int timeoutMs,
                                   VariantVector_t&& args);

    /**
     * @brief Trigger a transition in the HSM and process it synchronously.
     * @details Convenience wrapper for transitionEx() which tries to execute transition synchronously. Please see
     * transitionEx() for detailed description.
     *
     * @warning calling this function from HSM callback might result in a deadlock (see transitionEx() for details).
     *
     * @param event ID of event to send to HSM
     * @param timeoutMs maximum time in milliseconds to wait for the transition to complete if sync is true. Use
     * HSM_WAIT_INDEFINITELY to wait indefinitely.
     * @param args (optional) arguments to pass to the callbacks
     *
     * @retval true event was accepted and transition successfully finished
     * @retval false no matching transitions were found, transition was canceled or timeoutMs expired
     *
     * @threadsafe{ }
     */
    template <typename... Args>
    bool transitionSync(const EventID_t event, const int timeoutMs, Args&&... args);

    /**
     * @brief Trigger a transition in the HSM and clear all pending events.
     * @details Convenience wrapper for transitionEx() which clears the pending events queue before sending a new event.
     * Transition is executed asynchronously.
     *
     * @param event ID of event to send to HSM
     * @param args (optional) arguments to pass to the callbacks
     *
     * @threadsafe{ }
     */
    template <typename... Args>
    void transitionWithQueueClear(const EventID_t event, Args&&... args);

    /**
     * @brief Interrupt/signal safe version of transition
     * @details This is a simplified version of transition that can be safely used from an interrupt/signal. Event is processed
     * asynchronously.
     *
     * @remark There are no restrictions to use other transition APIs inside an interrupt, but all of them use dynamic
     * heap memory allocation (which can cause heap corruption on some platfroms). This version of the transition relies on
     * dispatcher implementation and might not be available everywhere (please check dispatcher's description). It also might
     * fail if internal dispatcher events queue is full.
     *
     * @param event ID of event to send to HSM
     *
     * @retval true event was added to queue
     * @retval false failed to add event to queue because it's not supported by dispatcher or queue limit was reached
     *
     * @concurrencysafe{ }
     */
    bool transitionInterruptSafe(const EventID_t event);

    /**
     * @brief Check if a transition is possible.
     * @details This function checks if a transition can be triggered by the given event and current state.
     * This function takes a variable number of arguments, which will be passed to the condition callbacks of the relevant
     * states and transitions.
     *
     * @remark It's recommended to avoid using this API unless really needed. It might confuse in a multithreaded
     * environment since it only can check possibility of transition in the **current** HSM state, but it can't prevent this
     * state from changing after returning from isTransitionPossible(). You would have to use additional synchronization
     * mechanisms to guarantee that state doesn't change between calls to isTransitionPossible() and transition().
     *
     * @param event ID of event to send to HSM
     * @param args (optional) arguments to pass to the condition callbacks
     * @return True if a transition is possible, false otherwise.
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    template <typename... Args>
    bool isTransitionPossible(const EventID_t event, Args&&... args);

    /**
     * @brief Start a timer.
     * @details If timer with this ID is already running it will be restarted with new settings.
     *
     * @param timerID       unique timer id
     * @param intervalMs    timer interval in milliseconds
     * @param isSingleShot  true - timer will run only once and then will stop
     *                      false - timer will keep running until stopTimer() is called or dispatcher is destroyed
     *
     * @threadsafe{ }
     */
    void startTimer(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot);

    /**
     * @brief Restart running timer.
     * @details Timer is restarted with the same arguments which were provided to startTimer(). Only currently running or
     * expired timers (with isSingleShot set to true) will be restarted. Has no effect if called for a timer which was not
     * started.
     *
     * @param timerID       id of running timer
     *
     * @threadsafe{ }
     */
    void restartTimer(const TimerID_t timerID);

    /**
     * @brief Stop active timer.
     * @details Function stops an active timer without triggering any notifications and unregisters it. Further calls to
     * restartTimer() will have no effects untill it's started again with startTimer().
     *
     * @remark For expired timers (which have isSingleShot property set to true), funtion simply unregisters them.
     *
     * @param timerID id of running or expired timer
     *
     * @threadsafe{ }
     */
    void stopTimer(const TimerID_t timerID);

    /**
     * @brief Check if timer is currently running.
     *
     * @param timerID id of the timer to check
     *
     * @retval true timer is running
     * @retval false timer is not running
     *
     * @threadsafe{ }
     */
    bool isTimerRunning(const TimerID_t timerID);

    /**
     * @brief Enable debugging for HSM instance.
     * @details Enables creation of the log file that can be later analyzed with hsmdebugger. By default log will be written to
     * ./dump.hsmlog file. This location can be overwritten by setting ENV_DUMPPATH environment variable with desired path.
     *
     * @remark HSMBUILD_DEBUGGING build option must be enabled for this functionality to work. It's recommended to keep this
     * feature disabled in production code to avoid performance overhead.
     *
     * @retval true debugging was successfully enabled
     * @retval false failed to open log file
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    bool enableHsmDebugging();

    /**
     * @brief Enable debugging for HSM instance with specific path for the log file.
     * Enables creation of the log file that can be later analyzed with hsmdebugger.
     *
     * @remark HSMBUILD_DEBUGGING build option must be enabled for this functionality to work. It's recommended to keep this
     * feature disabled in production code to avoid performance overhead.
     *
     * @param dumpPath The path for the dump files.
     * @retval true debugging was successfully enabled (always returns true if HSMBUILD_DEBUGGING was not set)
     * @retval false failed to open log file
     *
     * @notthreadsafe{Calling thing API from multiple threads can cause data races and will result in undefined behavior}
     */
    bool enableHsmDebugging(const std::string& dumpPath);

    /**
     * @brief Disable HSM debugging.
     * @details This function disables debugging for the Hierarchical State Machine and closes the log file.
     * Does nothing if enableHsmDebugging() was not called.
     *
     * @threadsafe{ Internally just calls std::filebuf::close(). }
     */
    void disableHsmDebugging();

protected:
    /**
     * @brief Convert state ID to a text name.
     * @details Default implementation only converts numeric state ID to a string. When scxml code generation is used, method
     * implementation of getStateName() will be generated. If HSM structure is registered manually it's developer's
     * responsibility to provide implementation of this method.
     *
     * @remark This method must be implemented for debugging to work. State names should match with the names in scxml file.
     *
     * @param state ID of the state
     * @return name of the state with requested ID or an ID converted to string
     *
     * @threadsafe{ }
     */
    virtual std::string getStateName(const StateID_t state) const;

    /**
     * @brief Convert event ID to a text name.
     * @details Default implementation only converts numeric event ID to a string. When scxml code generation is used, method
     * implementation of getEventName() will be generated. If HSM structure is registered manually it's developer's
     * responsibility to provide implementation of this method.
     *
     * @remark This method must be implemented for debugging to work. Event names should match with the names in scxml file.
     *
     * @param event ID of the event
     * @return name of the event with requested ID or an ID converted to string
     *
     * @threadsafe{ }
     */
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
    std::shared_ptr<Impl> mImpl;
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
                                                   Args&&... args) {
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, std::forward<Args>(args)...);
    return registerStateActionImpl(state, actionTrigger, action, eventArgs);
}

template <class HsmHandlerClass>
void HierarchicalStateMachine::registerTransition(const StateID_t fromState,
                                                  const StateID_t toState,
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

    registerTransition(fromState, toState, onEvent, funcTransitionCallback, funcConditionCallback, expectedConditionValue);
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
void HierarchicalStateMachine::transition(const EventID_t event, Args&&... args) {
    (void)transitionEx(event, false, false, 0, std::forward<Args>(args)...);
}

template <typename... Args>
bool HierarchicalStateMachine::transitionEx(const EventID_t event,
                                            const bool clearQueue,
                                            const bool sync,
                                            const int timeoutMs,
                                            Args&&... args) {
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, std::forward<Args>(args)...);
    return transitionExWithArgsArray(event, clearQueue, sync, timeoutMs, std::move(eventArgs));
}

template <typename... Args>
bool HierarchicalStateMachine::transitionSync(const EventID_t event, const int timeoutMs, Args&&... args) {
    return transitionEx(event, false, true, timeoutMs, std::forward<Args>(args)...);
}

template <typename... Args>
void HierarchicalStateMachine::transitionWithQueueClear(const EventID_t event, Args&&... args) {
    // NOTE: async transitions always return true
    (void)transitionEx(event, true, false, 0, std::forward<Args>(args)...);
}

template <typename... Args>
bool HierarchicalStateMachine::isTransitionPossible(const EventID_t event, Args&&... args) {
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, std::forward<Args>(args)...);

    return isTransitionPossibleImpl(event, eventArgs);
}

template <typename... Args>
void HierarchicalStateMachine::makeVariantList(VariantVector_t& vList, Args&&... args) {
    volatile int make_variant[] = {0, (vList.emplace_back(std::forward<Args>(args)), 0)...};
    (void)make_variant;
}

}  // namespace hsmcpp

#endif  // HSMCPP_HSM_HPP
