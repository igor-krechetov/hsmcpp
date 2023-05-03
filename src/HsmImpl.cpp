// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "HsmImpl.hpp"

#include <algorithm>

#include "hsmcpp/IHsmEventDispatcher.hpp"
#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/os.hpp"
#include "hsmcpp/os/LockGuard.hpp"
#include "hsmcpp/os/ConditionVariable.hpp"

#if !defined(HSM_DISABLE_THREADSAFETY) && defined(FREERTOS_AVAILABLE)
  #include "hsmcpp/os/InterruptsFreeSection.hpp"
#endif

#ifdef HSMBUILD_DEBUGGING
  #include <chrono>
  #include <cstdlib>
  #include <cstring>

// WIN, access
  #ifdef WIN32
    #include <io.h>
    #define F_OK 0
  #else
    #include <unistd.h>
    // cppcheck-suppress misra-c2012-21.10
    #include <ctime>
  #endif
#endif

#ifndef HSM_DISABLE_DEBUG_TRACES
  #define DEBUG_DUMP_ACTIVE_STATES() dumpActiveStates()
#else
  #define DEBUG_DUMP_ACTIVE_STATES()
#endif

// NOTE: used only for logging during development. in release mode macros is empty
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// cppcheck-suppress misra-c2012-8.4
HSM_TRACE_PREINIT()
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

constexpr const char* HSM_TRACE_CLASS = "HierarchicalStateMachine";

// These macroses can't be converted to 'constexpr' template functions
// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// If defined, HSM will performe safety checks during states and substates registration.
// Normally HSM structure should be static, so this feature is usefull only
// during development since it reduces performance a bit
// #define HSM_ENABLE_SAFE_STRUCTURE                    1

// Thread safety is enabled by default, but it adds some overhead related with mutex usage.
// If performance is critical and it's ensured that HSM is used only from a single thread,
// then synchronization could be disabled during compilation.
// #define HSM_DISABLE_THREADSAFETY                     1
#ifdef HSM_DISABLE_THREADSAFETY
  #define HSM_SYNC_EVENTS_QUEUE()
#elif defined(FREERTOS_AVAILABLE)
  #define HSM_SYNC_EVENTS_QUEUE() InterruptsFreeSection lck
#else
  #define HSM_SYNC_EVENTS_QUEUE() LockGuard lck(mEventsSync)
#endif  // HSM_DISABLE_THREADSAFETY

// NOLINTEND(cppcoreguidelines-macro-usage)

namespace hsmcpp {

// ============================================================================
// PUBLIC
// ============================================================================
HierarchicalStateMachine::Impl::Impl(HierarchicalStateMachine* parent, const StateID_t initialState)
    // cppcheck-suppress misra-c2012-10.4 ; false-positive. thinks that ':' is arithmetic operation
    : mParent(parent)
    , mInitialState(initialState) {
    HSM_TRACE_INIT();
}

HierarchicalStateMachine::Impl::~Impl() {
    release();
}

void HierarchicalStateMachine::Impl::resetParent() {
#ifndef HSM_DISABLE_THREADSAFETY
    LockGuard lk(mParentSync);
#endif

    mParent = nullptr;
}

void HierarchicalStateMachine::Impl::setInitialState(const StateID_t initialState) {
    if (true == mDispatcher.expired()) {
        mInitialState = initialState;
    }
}

bool HierarchicalStateMachine::Impl::initialize(const std::weak_ptr<IHsmEventDispatcher>& dispatcher) {
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    if (true == mDispatcher.expired()) {
        auto dispatcherPtr = dispatcher.lock();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (dispatcherPtr) {
            if (true == dispatcherPtr->start()) {
                std::weak_ptr<Impl> ptrInstance;

                HSM_TRY {
                    ptrInstance = shared_from_this();
                }
                HSM_CATCH(const std::bad_weak_ptr& e) {
                    HSM_TRACE_ERROR("Impl instance must be created as shared_ptr");
                }

                if (false == ptrInstance.expired()) {
                    mDispatcher = dispatcher;

                    createEventHandler(dispatcherPtr, ptrInstance);
                    createTimerHandler(dispatcherPtr, ptrInstance);
                    createEnqueuedEventHandler(dispatcherPtr, ptrInstance);

                    if ((INVALID_HSM_DISPATCHER_HANDLER_ID != mEventsHandlerId) &&
                        (INVALID_HSM_DISPATCHER_HANDLER_ID != mTimerHandlerId)) {
                        logHsmAction(HsmLogAction::IDLE,
                                     INVALID_HSM_STATE_ID,
                                     INVALID_HSM_STATE_ID,
                                     INVALID_HSM_EVENT_ID,
                                     false,
                                     VariantVector_t());
                        handleStartup();
                        result = true;
                    } else {
                        HSM_TRACE_ERROR("failed to register event handlers");
                        dispatcherPtr->unregisterEventHandler(mEventsHandlerId);
                        dispatcherPtr->unregisterEnqueuedEventHandler(mEnqueuedEventsHandlerId);
                        dispatcherPtr->unregisterTimerHandler(mTimerHandlerId);
                    }
                }
            } else {
                HSM_TRACE_ERROR("failed to start dispatcher");
            }
        } else {
            HSM_TRACE_ERROR("dispatcher is NULL");
        }
    } else {
        HSM_TRACE_ERROR("already initialized");
    }

    return result;
}

bool HierarchicalStateMachine::Impl::isInitialized() const {
    return (false == mDispatcher.expired());
}

void HierarchicalStateMachine::Impl::release() {
    mStopDispatching = true;
    HSM_TRACE_CALL_DEBUG();

    disableHsmDebugging();

    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        dispatcherPtr->unregisterEventHandler(mEventsHandlerId);
        dispatcherPtr->unregisterEnqueuedEventHandler(mEnqueuedEventsHandlerId);
        dispatcherPtr->unregisterTimerHandler(mTimerHandlerId);
        mDispatcher.reset();
        mEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    }
}

void HierarchicalStateMachine::Impl::registerFailedTransitionCallback(HsmTransitionFailedCallback_t onFailedTransition) {
    mFailedTransitionCallback = std::move(onFailedTransition);
}

void HierarchicalStateMachine::Impl::registerState(const StateID_t state,
                                                   HsmStateChangedCallback_t onStateChanged,
                                                   HsmStateEnterCallback_t onEntering,
                                                   HsmStateExitCallback_t onExiting) {
#ifdef HSM_ENABLE_SAFE_STRUCTURE
    if ((false == isSubstate(state)) && (false == isTopState(state))) {
        mTopLevelStates.push_back(state);
    }
#endif  // HSM_ENABLE_SAFE_STRUCTURE

    if (onStateChanged || onEntering || onExiting) {
        StateCallbacks cbState;

        cbState.onStateChanged = std::move(onStateChanged);
        cbState.onEntering = std::move(onEntering);
        cbState.onExiting = std::move(onExiting);
        mRegisteredStates[state] = cbState;

        HSM_TRACE_CALL_DEBUG_ARGS("mRegisteredStates.size=%ld", mRegisteredStates.size());
    }
}

void HierarchicalStateMachine::Impl::registerFinalState(const StateID_t state,
                                                        const EventID_t event,
                                                        HsmStateChangedCallback_t onStateChanged,
                                                        HsmStateEnterCallback_t onEntering,
                                                        HsmStateExitCallback_t onExiting) {
    mFinalStates.emplace(state, event);
    registerState(state, std::move(onStateChanged), std::move(onEntering), std::move(onExiting));
}

void HierarchicalStateMachine::Impl::registerHistory(const StateID_t parent,
                                                     const StateID_t historyState,
                                                     const HistoryType type,
                                                     const StateID_t defaultTarget,
                                                     HsmTransitionCallback_t transitionCallback) {
    (void)mHistoryStates.emplace(parent, historyState);
    mHistoryData.emplace(historyState, HistoryInfo(type, defaultTarget, std::move(transitionCallback)));
}

bool HierarchicalStateMachine::Impl::registerSubstate(const StateID_t parent, const StateID_t substate) {
    return registerSubstate(parent, substate, false);
}

bool HierarchicalStateMachine::Impl::registerSubstateEntryPoint(const StateID_t parent,
                                                                const StateID_t substate,
                                                                const EventID_t onEvent,
                                                                HsmTransitionConditionCallback_t conditionCallback,
                                                                const bool expectedConditionValue) {
    return registerSubstate(parent, substate, true, onEvent, std::move(conditionCallback), expectedConditionValue);
}

void HierarchicalStateMachine::Impl::registerTimer(const TimerID_t timerID, const EventID_t event) {
    mTimers.emplace(timerID, event);
}

bool HierarchicalStateMachine::Impl::registerSubstate(const StateID_t parent,
                                                      const StateID_t substate,
                                                      const bool isEntryPoint,
                                                      const EventID_t eventCondition,
                                                      HsmTransitionConditionCallback_t conditionCallback,
                                                      const bool expectedConditionValue) {
    bool registrationAllowed = false;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    // do a simple sanity check
    if (parent != substate) {
        StateID_t curState = parent;
        StateID_t prevState = INVALID_HSM_STATE_ID;

        if (false == hasParentState(substate, prevState)) {
            registrationAllowed = true;

            while (true == hasParentState(curState, prevState)) {
                if (substate == prevState) {
                    HSM_TRACE_CALL_DEBUG_ARGS(
                        "requested operation will result in substates recursion (parent=<%s>, substate=<%s>)",
                        getStateName(parent).c_str(),
                        getStateName(substate).c_str());
                    registrationAllowed = false;
                    break;
                }

                curState = prevState;
            }
        } else {
            HSM_TRACE_CALL_DEBUG_ARGS("substate <%s> already has a parent <%s>",
                                      getStateName(substate).c_str(),
                                      getStateName(prevState).c_str());
        }
    }
#else
    registrationAllowed = (parent != substate);
#endif  // HSM_ENABLE_SAFE_STRUCTURE

    if (registrationAllowed) {
        // NOTE: false-positive. isEntryPoint is of type bool
        // cppcheck-suppress misra-c2012-14.4
        if (isEntryPoint) {
            StateEntryPoint entryInfo;

            entryInfo.state = substate;
            entryInfo.onEvent = eventCondition;
            entryInfo.checkCondition = std::move(conditionCallback);
            entryInfo.expectedConditionValue = expectedConditionValue;

            (void)mSubstateEntryPoints.emplace(parent, entryInfo);
        }

        (void)mSubstates.emplace(parent, substate);

#ifdef HSM_ENABLE_SAFE_STRUCTURE
        if (true == isTopState(substate)) {
            mTopLevelStates.remove(substate);
        }
#endif  // HSM_ENABLE_SAFE_STRUCTURE
    }

    return registrationAllowed;
}

bool HierarchicalStateMachine::Impl::registerStateAction(const StateID_t state,
                                                         const StateActionTrigger actionTrigger,
                                                         const StateAction action,
                                                         const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>, actionTrigger=%d, action=%d",
                              getStateName(state).c_str(),
                              SC2INT(actionTrigger),
                              SC2INT(action));
    bool result = false;
    bool argsValid = false;
    StateActionInfo newAction;

    newAction.actionArgs = args;

    // validate arguments
    switch (action) {
        case StateAction::START_TIMER:
            argsValid = (newAction.actionArgs.size() == 3U) && newAction.actionArgs[0].isNumeric() &&
                        newAction.actionArgs[1].isNumeric() && newAction.actionArgs[2].isBool();
            break;
        case StateAction::RESTART_TIMER:
        case StateAction::STOP_TIMER:
            argsValid = (newAction.actionArgs.size() == 1U) && newAction.actionArgs[0].isNumeric();
            break;
        case StateAction::TRANSITION:
            argsValid = (false == newAction.actionArgs.empty()) && newAction.actionArgs[0].isNumeric();
            break;
        default:
            // do nothing
            break;
    }

    if (true == argsValid) {
        newAction.action = action;
        (void)mRegisteredActions.emplace(std::make_pair(state, actionTrigger), newAction);
        result = true;
    } else {
        HSM_TRACE_ERROR("invalid arguments");
    }

    return result;
}

void HierarchicalStateMachine::Impl::registerTransition(const StateID_t fromState,
                                                        const StateID_t toState,
                                                        const EventID_t onEvent,
                                                        HsmTransitionCallback_t transitionCallback,
                                                        HsmTransitionConditionCallback_t conditionCallback,
                                                        const bool expectedConditionValue) {
    (void)mTransitionsByEvent.emplace(std::make_pair(fromState, onEvent),
                                      TransitionInfo(fromState,
                                                     toState,
                                                     TransitionType::EXTERNAL_TRANSITION,
                                                     std::move(transitionCallback),
                                                     std::move(conditionCallback),
                                                     expectedConditionValue));
}

void HierarchicalStateMachine::Impl::registerSelfTransition(const StateID_t state,
                                                            const EventID_t onEvent,
                                                            const TransitionType type,
                                                            HsmTransitionCallback_t transitionCallback,
                                                            HsmTransitionConditionCallback_t conditionCallback,
                                                            const bool expectedConditionValue) {
    (void)mTransitionsByEvent.emplace(
        std::make_pair(state, onEvent),
        TransitionInfo(state, state, type, std::move(transitionCallback), std::move(conditionCallback), expectedConditionValue));
}

StateID_t HierarchicalStateMachine::Impl::getLastActiveState() const {
    StateID_t currentState = INVALID_HSM_STATE_ID;

    if (false == mActiveStates.empty()) {
        currentState = mActiveStates.back();
    }

    return currentState;
}

const std::list<StateID_t>& HierarchicalStateMachine::Impl::getActiveStates() const {
    return mActiveStates;
}

bool HierarchicalStateMachine::Impl::isStateActive(const StateID_t state) const {
    return (std::find(mActiveStates.begin(), mActiveStates.end(), state) != mActiveStates.end());
}

void HierarchicalStateMachine::Impl::transitionWithArgsArray(const EventID_t event, const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, args.size=%lu", getEventName(event).c_str(), args.size());

    (void)transitionExWithArgsArray(event, false, false, 0, args);
}

bool HierarchicalStateMachine::Impl::transitionExWithArgsArray(const EventID_t event,
                                                               const bool clearQueue,
                                                               const bool sync,
                                                               const int timeoutMs,
                                                               const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, clearQueue=%s, sync=%s, args.size=%lu",
                              getEventName(event).c_str(),
                              BOOL2STR(clearQueue),
                              BOOL2STR(sync),
                              args.size());

    bool status = false;
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        PendingEventInfo eventInfo;

        eventInfo.type = event;
        eventInfo.args = args;

        if (true == sync) {
            eventInfo.initLock();
        }

        {
            HSM_SYNC_EVENTS_QUEUE();

            if (true == clearQueue) {
                clearPendingEvents();
            }

            mPendingEvents.push_back(eventInfo);
        }

        HSM_TRACE_DEBUG("transitionEx: emit");
        dispatcherPtr->emitEvent(mEventsHandlerId);

        if (true == sync) {
            HSM_TRACE_DEBUG("transitionEx: wait...");
            eventInfo.wait(timeoutMs);
            status = (HsmEventStatus::DONE_OK == *eventInfo.transitionStatus);
        } else {
            // always return true for async transitions
            status = true;
        }
    } else {
        HSM_TRACE_ERROR("HSM is not initialized");
    }

    return status;
}

bool HierarchicalStateMachine::Impl::transitionInterruptSafe(const EventID_t event) {
    bool res = false;
    // TODO: this part needs testing with real interrupts. Not sure if it's safe to use weak_ptr.lock()
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        res = dispatcherPtr->enqueueEvent(mEnqueuedEventsHandlerId, event);
    }

    return res;
}

bool HierarchicalStateMachine::Impl::isTransitionPossible(const EventID_t event, const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());
    bool possible = false;

    for (const StateID_t& state : mActiveStates) {
        possible = checkTransitionPossibility(state, event, args);

        if (true == possible) {
            break;
        }
    }

    HSM_TRACE_CALL_RESULT("%d", BOOL2INT(possible));
    return possible;
}

void HierarchicalStateMachine::Impl::startTimer(const TimerID_t timerID,
                                                const unsigned int intervalMs,
                                                const bool isSingleShot) {
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        dispatcherPtr->startTimer(mTimerHandlerId, timerID, intervalMs, isSingleShot);
    }
}

void HierarchicalStateMachine::Impl::restartTimer(const TimerID_t timerID) {
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        dispatcherPtr->restartTimer(timerID);
    }
}

void HierarchicalStateMachine::Impl::stopTimer(const TimerID_t timerID) {
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        dispatcherPtr->stopTimer(timerID);
    }
}

bool HierarchicalStateMachine::Impl::isTimerRunning(const TimerID_t timerID) {
    bool running = false;
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        running = dispatcherPtr->isTimerRunning(timerID);
    }

    return running;
}

// ============================================================================
// PRIVATE
// ============================================================================
void HierarchicalStateMachine::Impl::createEventHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr,
                                                        const std::weak_ptr<HierarchicalStateMachine::Impl>& ptrInstance) {
    // cppcheck-suppress misra-c2012-13.1 ; false-positive. this is a functor, not initializer list
    mEventsHandlerId = dispatcherPtr->registerEventHandler([ptrInstance]() {
        bool handlerIsValid = false;
        auto pThis = ptrInstance.lock();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (pThis && (false == pThis->mStopDispatching)) {
            pThis->dispatchEvents();
            handlerIsValid = true;
        }

        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return handlerIsValid;
    });
}

void HierarchicalStateMachine::Impl::createTimerHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr,
                                                        const std::weak_ptr<HierarchicalStateMachine::Impl>& ptrInstance) {
    // cppcheck-suppress misra-c2012-13.1 ; false-positive. this is a functor, not initializer list
    mTimerHandlerId = dispatcherPtr->registerTimerHandler([ptrInstance](const TimerID_t timerId) {
        bool handlerIsValid = false;
        auto pThis = ptrInstance.lock();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (pThis && (false == pThis->mStopDispatching)) {
            pThis->dispatchTimerEvent(timerId);
            handlerIsValid = true;
        }

        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return handlerIsValid;
    });
}

void HierarchicalStateMachine::Impl::createEnqueuedEventHandler(
    const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr,
    const std::weak_ptr<HierarchicalStateMachine::Impl>& ptrInstance) {
    // cppcheck-suppress misra-c2012-13.1 ; false-positive. this is a functor, not initializer list
    mEnqueuedEventsHandlerId = dispatcherPtr->registerEnqueuedEventHandler([ptrInstance](const EventID_t event) {
        bool handlerIsValid = false;
        auto pThis = ptrInstance.lock();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (pThis && (false == pThis->mStopDispatching)) {
            pThis->transitionSimple(event);
            handlerIsValid = true;
        }

        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return handlerIsValid;
    });
}

void HierarchicalStateMachine::Impl::handleStartup() {
    HSM_TRACE_CALL_DEBUG_ARGS("mActiveStates.size=%ld", mActiveStates.size());
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        HSM_TRACE_DEBUG("state=<%s>", getStateName(mInitialState).c_str());
        std::list<StateID_t> entryPoints;

        (void)onStateEntering(mInitialState, VariantVector_t());
        mActiveStates.push_back(mInitialState);
        onStateChanged(mInitialState, VariantVector_t());

        if (true == getEntryPoints(mInitialState, INVALID_HSM_EVENT_ID, VariantVector_t(), entryPoints)) {
            PendingEventInfo entryPointTransitionEvent;

            entryPointTransitionEvent.transitionType = TransitionBehavior::ENTRYPOINT;
            entryPointTransitionEvent.type = INVALID_HSM_EVENT_ID;

            {
                HSM_SYNC_EVENTS_QUEUE();
                mPendingEvents.push_front(entryPointTransitionEvent);
            }
        }

        if (false == mPendingEvents.empty()) {
            dispatcherPtr->emitEvent(mEventsHandlerId);
        }
    }
}

void HierarchicalStateMachine::Impl::transitionSimple(const EventID_t event) {
    (void)transitionExWithArgsArray(event, false, false, 0, VariantVector_t());
}

void HierarchicalStateMachine::Impl::dispatchEvents() {
    HSM_TRACE_CALL_DEBUG_ARGS("mPendingEvents.size=%ld", mPendingEvents.size());
    auto dispatcherPtr = mDispatcher.lock();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr) {
        if (false == mStopDispatching) {
            if (false == mPendingEvents.empty()) {
                PendingEventInfo pendingEvent;

                {
                    HSM_SYNC_EVENTS_QUEUE();
                    pendingEvent = mPendingEvents.front();
                    mPendingEvents.pop_front();
                }

                HsmEventStatus transitiontStatus = doTransition(pendingEvent);

                HSM_TRACE_DEBUG("unlock with status %d", SC2INT(transitiontStatus));
                pendingEvent.unlock(transitiontStatus);
            }

            if ((false == mStopDispatching) && (false == mPendingEvents.empty())) {
                dispatcherPtr->emitEvent(mEventsHandlerId);
            }
        }
    }
}

void HierarchicalStateMachine::Impl::dispatchTimerEvent(const TimerID_t id) {
    HSM_TRACE_CALL_DEBUG_ARGS("id=%d", SC2INT(id));
    auto it = mTimers.find(id);

    if (mTimers.end() != it) {
        transitionSimple(it->second);
    }
}

bool HierarchicalStateMachine::Impl::onStateExiting(const StateID_t state) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", getStateName(state).c_str());
    bool res = true;
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onExiting) {
        res = it->second.onExiting();
        logHsmAction(HsmLogAction::CALLBACK_EXIT,
                     state,
                     INVALID_HSM_STATE_ID,
                     INVALID_HSM_EVENT_ID,
                     (false == res),
                     VariantVector_t());
    }

    // execute state action only if transition was accepted by client
    if (true == res) {
        executeStateAction(state, StateActionTrigger::ON_STATE_EXIT);
    }

    return res;
}

bool HierarchicalStateMachine::Impl::onStateEntering(const StateID_t state, const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", getStateName(state).c_str());
    bool res = true;

    // since we can have a situation when same state is entered twice (parallel transitions) there
    // is no need to call callbacks multiple times
    if (false == isStateActive(state)) {
        auto it = mRegisteredStates.find(state);

        if ((mRegisteredStates.end() != it) && it->second.onEntering) {
            res = it->second.onEntering(args);
            logHsmAction(HsmLogAction::CALLBACK_ENTER, INVALID_HSM_STATE_ID, state, INVALID_HSM_EVENT_ID, (false == res), args);
        }

        // execute state action only if transition was accepted by client
        if (true == res) {
            executeStateAction(state, StateActionTrigger::ON_STATE_ENTRY);
        }
    }

    return res;
}

void HierarchicalStateMachine::Impl::onStateChanged(const StateID_t state, const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", getStateName(state).c_str());
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onStateChanged) {
        it->second.onStateChanged(args);
        logHsmAction(HsmLogAction::CALLBACK_STATE, INVALID_HSM_STATE_ID, state, INVALID_HSM_EVENT_ID, false, args);
    } else {
        HSM_TRACE_WARNING("no callback registered for state <%s>", getStateName(state).c_str());
    }
}

void HierarchicalStateMachine::Impl::executeStateAction(const StateID_t state, const StateActionTrigger actionTrigger) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>, actionTrigger=%d", getStateName(state).c_str(), SC2INT(actionTrigger));
    auto dispatcherPtr = mDispatcher.lock();
    auto itRange = mRegisteredActions.equal_range(std::make_pair(state, actionTrigger));

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
    if (dispatcherPtr && (itRange.first != itRange.second)) {
        switch (actionTrigger) {
            case StateActionTrigger::ON_STATE_ENTRY:
                logHsmAction(HsmLogAction::ON_ENTER_ACTIONS, INVALID_HSM_STATE_ID, state);
                break;
            case StateActionTrigger::ON_STATE_EXIT:
                logHsmAction(HsmLogAction::ON_EXIT_ACTIONS, INVALID_HSM_STATE_ID, state);
                break;
            default:
                // NOTE: do nothing
                break;
        }

        for (auto it = itRange.first; it != itRange.second; ++it) {
            const StateActionInfo& actionInfo = it->second;

            if (StateAction::START_TIMER == actionInfo.action) {
                dispatcherPtr->startTimer(mTimerHandlerId,
                                          static_cast<TimerID_t>(actionInfo.actionArgs[0].toInt64()),
                                          actionInfo.actionArgs[1].toInt64(),
                                          actionInfo.actionArgs[2].toBool());
            } else if (StateAction::STOP_TIMER == actionInfo.action) {
                dispatcherPtr->stopTimer(static_cast<TimerID_t>(actionInfo.actionArgs[0].toInt64()));
            } else if (StateAction::RESTART_TIMER == actionInfo.action) {
                dispatcherPtr->restartTimer(static_cast<TimerID_t>(actionInfo.actionArgs[0].toInt64()));
            } else if (StateAction::TRANSITION == actionInfo.action) {
                VariantVector_t transitionArgs;

                if (actionInfo.actionArgs.size() > 1U) {
                    transitionArgs.reserve(actionInfo.actionArgs.size() - 1U);

                    for (size_t i = 1; i < actionInfo.actionArgs.size(); ++i) {
                        transitionArgs.push_back(actionInfo.actionArgs[i]);
                    }
                }

                transitionWithArgsArray(static_cast<EventID_t>(actionInfo.actionArgs[0].toInt64()), transitionArgs);
            } else {
                HSM_TRACE_WARNING("unsupported action <%d>", SC2INT(actionInfo.action));
            }
        }
    }
}

bool HierarchicalStateMachine::Impl::getParentState(const StateID_t child, StateID_t& outParent) {
    bool wasFound = false;
    auto it = std::find_if(mSubstates.begin(), mSubstates.end(), [child](const std::pair<StateID_t, StateID_t>& item) {
        // cppcheck-suppress misra-c2012-15.5 ; false-positive. "return" statement belongs to lambda function
        return (child == item.second);
    });

    if (mSubstates.end() != it) {
        outParent = it->first;  // cppcheck-suppress misra-c2012-17.8 ; outParent is used to return result
        wasFound = true;
    }

    return wasFound;
}
bool HierarchicalStateMachine::Impl::isSubstateOf(const StateID_t parent, const StateID_t child) {
    HSM_TRACE_CALL_DEBUG_ARGS("parent=<%s>, child=<%s>", getStateName(parent).c_str(), getStateName(child).c_str());
    StateID_t curState = child;

    if (parent != child) {
        // TODO: can be optimized by checking siblings on each level

        do {
            if (false == getParentState(curState, curState)) {
                break;
            }
        } while (parent != curState);
    }

    return (parent != child) && (parent == curState);
}

bool HierarchicalStateMachine::Impl::isFinalState(const StateID_t state) const {
    return (mFinalStates.end() != mFinalStates.find(state));
}

bool HierarchicalStateMachine::Impl::hasActiveChildren(const StateID_t parent, const bool includeFinal) {
    HSM_TRACE_CALL_DEBUG_ARGS("parent=<%s>", getStateName(parent).c_str());
    bool res = false;

    for (const StateID_t& activeStateId : mActiveStates) {
        if ((true == includeFinal) || (false == isFinalState(activeStateId))) {
            if (isSubstateOf(parent, activeStateId)) {
                HSM_TRACE_DEBUG("parent=<%s> has <%s> active",
                                getStateName(parent).c_str(),
                                getStateName(activeStateId).c_str());
                res = true;
                break;
            }
        }
    }

    HSM_TRACE_CALL_RESULT("res=%d", BOOL2INT(res));
    return res;
}

bool HierarchicalStateMachine::Impl::getHistoryParent(const StateID_t historyState, StateID_t& outParent) {
    bool wasFound = false;
    auto it =
        std::find_if(mHistoryStates.begin(), mHistoryStates.end(), [historyState](const std::pair<StateID_t, StateID_t>& item) {
            // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
            // cppcheck-suppress misra-c2012-15.5
            return (historyState == item.second);
        });

    if (mHistoryStates.end() != it) {
        outParent = it->first;  // cppcheck-suppress misra-c2012-17.8 ; outParent is used to return result
        wasFound = true;
    }

    return wasFound;
}

void HierarchicalStateMachine::Impl::updateHistory(const StateID_t topLevelState, const std::list<StateID_t>& exitedStates) {
    HSM_TRACE_CALL_DEBUG_ARGS("topLevelState=<%s>, exitedStates.size=%ld",
                              getStateName(topLevelState).c_str(),
                              exitedStates.size());
    std::list<std::list<StateID_t>*> upatedHistory;

    for (const auto& activeState : exitedStates) {
        StateID_t curState = activeState;
        StateID_t parentState = INVALID_HSM_STATE_ID;

        // check if we have parent state
        while (true == getParentState(curState, parentState)) {
            HSM_TRACE_DEBUG("curState=<%s>, parentState=<%s>",
                            getStateName(curState).c_str(),
                            getStateName(parentState).c_str());
            auto itRange = mHistoryStates.equal_range(parentState);

            // check if parent state has any history states
            if (itRange.first != itRange.second) {
                HSM_TRACE_DEBUG("parent=<%s> has history items", getStateName(parentState).c_str());

                for (auto it = itRange.first; it != itRange.second; ++it) {
                    auto itCurHistory = mHistoryData.find(it->second);

                    if (itCurHistory != mHistoryData.end()) {
                        const auto itUpdatedHistory =
                            std::find(upatedHistory.begin(), upatedHistory.end(), &itCurHistory->second.previousActiveStates);

                        // check if this is the first time we are updating this history state
                        if (itUpdatedHistory == upatedHistory.end()) {
                            // clear previos history items
                            itCurHistory->second.previousActiveStates.clear();
                            // we store pointer to be able to identify if this history state was already cleared or not
                            upatedHistory.push_back(&(itCurHistory->second.previousActiveStates));
                        }

                        if (HistoryType::SHALLOW == itCurHistory->second.type) {
                            if (std::find(itCurHistory->second.previousActiveStates.begin(),
                                          itCurHistory->second.previousActiveStates.end(),
                                          curState) == itCurHistory->second.previousActiveStates.end()) {
                                HSM_TRACE_DEBUG("SHALLOW -> store state <%s> in history of parent <%s>",
                                                getStateName(curState).c_str(),
                                                getStateName(it->second).c_str());
                                itCurHistory->second.previousActiveStates.push_back(curState);
                            }
                        } else if (HistoryType::DEEP == itCurHistory->second.type) {
                            if (std::find(itCurHistory->second.previousActiveStates.begin(),
                                          itCurHistory->second.previousActiveStates.end(),
                                          activeState) == itCurHistory->second.previousActiveStates.end()) {
                                HSM_TRACE_DEBUG("DEEP -> store state <%s> in history of parent <%s>",
                                                getStateName(activeState).c_str(),
                                                getStateName(it->second).c_str());
                                itCurHistory->second.previousActiveStates.push_back(activeState);
                            }
                        } else {
                            // NOTE: do nothing
                        }
                    }
                }
            }

            if (topLevelState != parentState) {
                curState = parentState;
            } else {
                break;
            }
        }
    }
}

bool HierarchicalStateMachine::Impl::checkTransitionPossibility(const StateID_t fromState,
                                                                const EventID_t event,
                                                                const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());

    StateID_t currentState = fromState;
    std::list<TransitionInfo> possibleTransitions;
    EventID_t nextEvent = INVALID_HSM_EVENT_ID;
    bool possible = true;

    {
        // NOTE: findTransitionTarget can be a bit heavy. possible optimization to reduce lock time is
        //       to make a copy of mPendingEvents and work with it
        HSM_SYNC_EVENTS_QUEUE();

        for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) && (true == possible); ++it) {
            nextEvent = it->type;
            possible = findTransitionTarget(currentState, nextEvent, args, true, possibleTransitions);

            if (true == possible) {
                if (false == possibleTransitions.empty()) {
                    currentState = possibleTransitions.front().destinationState;
                } else {
                    possible = false;
                    break;
                }
            }
        }
    }

    if (true == possible) {
        nextEvent = event;
        possible = findTransitionTarget(currentState, nextEvent, args, true, possibleTransitions);
    }

    HSM_TRACE_CALL_RESULT("%d", BOOL2INT(possible));
    return possible;
}

bool HierarchicalStateMachine::Impl::findTransitionTarget(const StateID_t fromState,
                                                          const EventID_t event,
                                                          const VariantVector_t& transitionArgs,
                                                          const bool searchParents,
                                                          std::list<TransitionInfo>& outTransitions) {
    HSM_TRACE_CALL_DEBUG_ARGS("fromState=<%s>, event=<%s>", getStateName(fromState).c_str(), getEventName(event).c_str());
    bool continueSearch = false;
    StateID_t curState = fromState;

    do {
        const auto itRange = mTransitionsByEvent.equal_range(std::make_pair(curState, event));

        continueSearch = false;

        // if there are no matching transitions try to go one level up
        if (itRange.first == itRange.second) {
            if (true == searchParents) {
                continueSearch = getParentState(curState, curState);
            }

            continue;
        }

        // check available transitions
        for (auto it = itRange.first; it != itRange.second; ++it) {
            HSM_TRACE_DEBUG("check transition to <%s>...", getStateName(it->second.destinationState).c_str());

            if ((nullptr == it->second.checkCondition) ||
                (it->second.expectedConditionValue == it->second.checkCondition(transitionArgs))) {
                bool wasFound = false;
                std::list<StateID_t> parentStates = {it->second.destinationState};

                // cppcheck-suppress misra-c2012-15.4
                do {
                    StateID_t currentParent = parentStates.front();

                    parentStates.pop_front();

                    // if state has substates we must check if transition into them is possible (after cond)
                    if (true == hasSubstates(currentParent)) {
                        if (true == hasEntryPoint(currentParent)) {
                            HSM_TRACE_DEBUG("state <%s> has entrypoints", getStateName(currentParent).c_str());
                            std::list<StateID_t> entryPoints;

                            if (true == getEntryPoints(currentParent, event, transitionArgs, entryPoints)) {
                                parentStates.splice(parentStates.end(), entryPoints);
                            } else {
                                HSM_TRACE_WARNING("no matching entrypoints found");
                                break;
                            }
                        } else {
                            HSM_TRACE_WARNING("state <%s> doesn't have an entrypoint defined",
                                              getStateName(currentParent).c_str());
                            break;
                        }
                    } else {
                        outTransitions.push_back(it->second);
                        wasFound = true;
                    }
                } while ((false == wasFound) && (parentStates.empty() == false));
            }
        }
    } while (true == continueSearch);

    HSM_TRACE_CALL_RESULT("%s", BOOL2STR(outTransitions.empty() == false));
    return (outTransitions.empty() == false);
}

HsmEventStatus HierarchicalStateMachine::Impl::doTransition(const PendingEventInfo& event) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, transitionType=%d", getEventName(event.type).c_str(), SC2INT(event.transitionType));
    HsmEventStatus res = HsmEventStatus::DONE_FAILED;
    auto activeStatesSnapshot = mActiveStates;
    std::list<StateID_t> acceptedStates;  // list of states that accepted transitions

    for (auto it = activeStatesSnapshot.rbegin(); it != activeStatesSnapshot.rend(); ++it) {
        // in case of parallel transitions some states might become inactive after handleSingleTransition()
        // example: [*B, *C] -> D
        if (true == isStateActive(*it)) {
            // we don't need to process transitions for active states if their child already processed it
            bool childStateProcessed = false;

            for (const auto& state : acceptedStates) {
                if (true == isSubstateOf(*it, state)) {
                    childStateProcessed = true;
                    break;
                }
            }

            if (false == childStateProcessed) {
                const HsmEventStatus singleTransitionResult = handleSingleTransition(*it, event);

                switch (singleTransitionResult) {
                    case HsmEventStatus::PENDING:
                        res = singleTransitionResult;
                        acceptedStates.push_back(*it);
                        break;
                    case HsmEventStatus::DONE_OK:
                        logHsmAction(HsmLogAction::IDLE,
                                     INVALID_HSM_STATE_ID,
                                     INVALID_HSM_STATE_ID,
                                     INVALID_HSM_EVENT_ID,
                                     false,
                                     VariantVector_t());
                        if (HsmEventStatus::PENDING != res) {
                            res = singleTransitionResult;
                        }
                        acceptedStates.push_back(*it);
                        break;
                    case HsmEventStatus::CANCELED:
                    case HsmEventStatus::DONE_FAILED:
                    default:
                        // do nothing
                        break;
                }
            }
        }
    }

    if (mFailedTransitionCallback && ((HsmEventStatus::DONE_FAILED == res) || (HsmEventStatus::CANCELED == res))) {
        mFailedTransitionCallback(event.type, event.args);
    }

    HSM_TRACE_CALL_RESULT("%d", SC2INT(res));
    return res;
}

HsmEventStatus HierarchicalStateMachine::Impl::processExternalTransition(const PendingEventInfo& event,
                                                                           const StateID_t fromState,
                                                                           const TransitionInfo& curTransition,
                                                                           const std::list<StateID_t>& exitedStates) {
    HsmEventStatus res = HsmEventStatus::DONE_FAILED;

    // NOTE: Decide if we need functionality to cancel ongoing transition
    logHsmAction(((TransitionBehavior::ENTRYPOINT != event.transitionType) ? HsmLogAction::TRANSITION
                                                                           : HsmLogAction::TRANSITION_ENTRYPOINT),
                 curTransition.fromState,
                 curTransition.destinationState,
                 event.type,
                 false,
                 event.args);

    // cppcheck-suppress misra-c2012-14.4 : false-positive. std::shared_ptr has a bool() operator
    if (curTransition.onTransition) {
        curTransition.onTransition(event.args);
    }

    if (true == onStateEntering(curTransition.destinationState, event.args)) {
        if (true == replaceActiveState(fromState, curTransition.destinationState)) {
            onStateChanged(curTransition.destinationState, event.args);
        }

        // check if current state is a final state
        if (true == processFinalStateTransition(event, curTransition.destinationState)) {
            res = HsmEventStatus::DONE_OK;
        }
        // check if we transitioned into history state
        else if (true == processHistoryTransition(event, curTransition.destinationState)) {
            res = HsmEventStatus::PENDING;
        } else {
            // check if new state has substates and initiate entry transition
            if (false == event.ignoreEntryPoints) {
                std::list<StateID_t> entryPoints;

                if (true == getEntryPoints(curTransition.destinationState, event.type, event.args, entryPoints)) {
                    HSM_TRACE_DEBUG("state <%s> has substates with %d entry points (first: <%s>)",
                                    getStateName(curTransition.destinationState).c_str(),
                                    SC2INT(entryPoints.size()),
                                    getStateName(entryPoints.front()).c_str());
                    PendingEventInfo entryPointTransitionEvent = event;

                    entryPointTransitionEvent.transitionType = TransitionBehavior::ENTRYPOINT;

                    {
                        HSM_SYNC_EVENTS_QUEUE();
                        mPendingEvents.push_front(entryPointTransitionEvent);
                    }
                    res = HsmEventStatus::PENDING;
                } else {
                    res = HsmEventStatus::DONE_OK;
                }
            } else {
                HSM_TRACE_DEBUG("entry points were forcefully ignored (probably due to history transition)");
                res = HsmEventStatus::PENDING;
            }
        }
    } else {
        for (const StateID_t& curState : exitedStates) {
            // to prevent infinite loops we don't allow state to cancel transition
            (void)onStateEntering(curState, VariantVector_t());
            (void)addActiveState(curState);
            onStateChanged(curState, VariantVector_t());
        }
    }

    return res;
}

bool HierarchicalStateMachine::Impl::determineTargetState(const PendingEventInfo& event,
                                                          const StateID_t fromState,
                                                          std::list<TransitionInfo>& outMatchingTransitions) {
    bool isCorrectTransition = false;

    if (TransitionBehavior::REGULAR == event.transitionType) {
        isCorrectTransition = findTransitionTarget(fromState, event.type, event.args, false, outMatchingTransitions);

        if (false == isCorrectTransition) {
            HSM_TRACE_WARNING("no suitable transition from state <%s> with event <%s>",
                              getStateName(fromState).c_str(),
                              getEventName(event.type).c_str());
        }
    } else if (TransitionBehavior::ENTRYPOINT == event.transitionType) {
        isCorrectTransition = true;

        // if fromState doesnt have active children
        for (auto it = mActiveStates.rbegin(); it != mActiveStates.rend(); ++it) {
            if (fromState != *it) {
                StateID_t activeParent = INVALID_HSM_STATE_ID;

                if (true == getParentState(*it, activeParent)) {
                    if (activeParent == fromState) {
                        // no need to handle entry transition for already active state
                        isCorrectTransition = false;
                        break;
                    }
                }
            }
        }

        if (true == isCorrectTransition) {
            std::list<StateID_t> entryStates;

            isCorrectTransition = getEntryPoints(fromState, event.type, event.args, entryStates);

            if (true == isCorrectTransition) {
                for (const auto& curEntryState : entryStates) {
                    (void)outMatchingTransitions.emplace_back(fromState,
                                                              curEntryState,
                                                              TransitionType::EXTERNAL_TRANSITION,
                                                              nullptr,
                                                              nullptr);
                }
            } else {
                HSM_TRACE_WARNING("state <%s> doesn't have a suitable entry point (event <%s>)",
                                  getStateName(fromState).c_str(),
                                  getEventName(event.type).c_str());
            }
        }
    } else if (TransitionBehavior::FORCED == event.transitionType) {
        HSM_TRACE_DEBUG("forced history transitions: %d", SC2INT(event.forcedTransitionsInfo->size()));
        // cppcheck-suppress misra-c2012-17.8 ; outMatchingTransitions is used to return result
        outMatchingTransitions = *event.forcedTransitionsInfo;
        isCorrectTransition = true;
    } else {
        // NOTE: do nothing
    }

    return isCorrectTransition;
}

bool HierarchicalStateMachine::Impl::executeSelfTransitions(const PendingEventInfo& event,
                                                            const std::list<TransitionInfo>& matchingTransitions) {
    bool hadSelfTransitions = false;

    // execute self transitions first
    for (const auto& curTransition : matchingTransitions) {
        if ((curTransition.fromState == curTransition.destinationState) &&
            (TransitionType::INTERNAL_TRANSITION == curTransition.transitionType)) {
            // TODO: separate type for self transition?
            logHsmAction(HsmLogAction::TRANSITION,
                         curTransition.fromState,
                         curTransition.destinationState,
                         event.type,
                         false,
                         event.args);

            // NOTE: false-positive. std::function has a bool() operator
            // cppcheck-suppress misra-c2012-14.4
            if (curTransition.onTransition) {
                curTransition.onTransition(event.args);
            }

            hadSelfTransitions = true;
        }
    }

    return hadSelfTransitions;
}

bool HierarchicalStateMachine::Impl::executeExitTransition(const PendingEventInfo& event,
                                                           const std::list<TransitionInfo>& matchingTransitions,
                                                           std::list<StateID_t>& outExitedStates) {
    bool isExitAllowed = true;

    for (const auto& curTransition : matchingTransitions) {
        if (// process everything except internal self-transitions
            ((curTransition.fromState != curTransition.destinationState) ||
            (TransitionType::EXTERNAL_TRANSITION == curTransition.transitionType)) &&
            // exit active states only during regular transitions
            (TransitionBehavior::REGULAR == event.transitionType)) {

            // it's an outer transition from parent state. we need to find and exit all active substates
            for (auto itActiveState = mActiveStates.rbegin(); itActiveState != mActiveStates.rend(); ++itActiveState) {
                HSM_TRACE_DEBUG("OUTER EXIT: FROM=%s, ACTIVE=%s",
                                getStateName(curTransition.fromState).c_str(),
                                getStateName(*itActiveState).c_str());
                if ((curTransition.fromState == *itActiveState) ||
                    (true == isSubstateOf(curTransition.fromState, *itActiveState))) {
                    isExitAllowed = onStateExiting(*itActiveState);

                    if (true == isExitAllowed) {
                        outExitedStates.push_back(*itActiveState);
                    } else {
                        break;
                    }
                }
            }

            // if no one blocked ongoing transition - remove child states from active list
            if (true == isExitAllowed) {
                // store history for states between "fromState" ----> "it->fromState"
                updateHistory(curTransition.fromState, outExitedStates);

                for (const auto& curState : outExitedStates) {
                    mActiveStates.remove(curState);
                }
            }
            // if one of the states blocked ongoing transition we need to rollback
            else {
                for (const auto& curState : outExitedStates) {
                    mActiveStates.remove(curState);
                    // to prevent infinite loops we don't allow state to cancel transition
                    (void)onStateEntering(curState, VariantVector_t());
                    mActiveStates.push_back(curState);
                    onStateChanged(curState, VariantVector_t());
                }
            }
        }
    }

    return isExitAllowed;
}

bool HierarchicalStateMachine::Impl::processHistoryTransition(const PendingEventInfo& event, const StateID_t destinationState) {
    auto itHistoryData = mHistoryData.find(destinationState);

    // check if we transitioned into a history state
    if (itHistoryData != mHistoryData.end()) {
        HSM_TRACE_DEBUG("state=<%s> is a history state with %ld stored states",
                        getStateName(destinationState).c_str(),
                        itHistoryData->second.previousActiveStates.size());

        if (itHistoryData->second.previousActiveStates.empty() == false) {
            transitionToPreviousActiveStates(itHistoryData->second.previousActiveStates, event, destinationState);
        } else {
            transitionToDefaultHistoryState(itHistoryData->second.defaultTarget,
                                            itHistoryData->second.defaultTargetTransitionCallback,
                                            event,
                                            destinationState);
        }
    }

    return (itHistoryData != mHistoryData.end());
}

void HierarchicalStateMachine::Impl::transitionToPreviousActiveStates(std::list<StateID_t>& previousActiveStates,
                                                                      const PendingEventInfo& event,
                                                                      const StateID_t destinationState) {
    StateID_t prevChildState = INVALID_HSM_STATE_ID;
    PendingEventInfo historyTransitionEvent = event;

    historyTransitionEvent.transitionType = TransitionBehavior::FORCED;
    historyTransitionEvent.forcedTransitionsInfo = std::make_shared<std::list<TransitionInfo>>();

    {
        HSM_SYNC_EVENTS_QUEUE();

        for (const StateID_t prevState : previousActiveStates) {
            if ((INVALID_HSM_STATE_ID != prevChildState) && (true == isSubstateOf(prevState, prevChildState))) {
                if (false == historyTransitionEvent.forcedTransitionsInfo->empty()) {
                    mPendingEvents.push_front(historyTransitionEvent);
                }

                historyTransitionEvent.forcedTransitionsInfo = std::make_shared<std::list<TransitionInfo>>();
                historyTransitionEvent.ignoreEntryPoints = true;
            } else {
                historyTransitionEvent.ignoreEntryPoints = false;
            }

            prevChildState = prevState;
            historyTransitionEvent.forcedTransitionsInfo->emplace_back(destinationState,
                                                                       prevState,
                                                                       TransitionType::EXTERNAL_TRANSITION,
                                                                       nullptr,
                                                                       nullptr);
        }

        mPendingEvents.push_front(historyTransitionEvent);
    }

    previousActiveStates.clear();

    StateID_t historyParent = INVALID_HSM_STATE_ID;

    if (true == getHistoryParent(destinationState, historyParent)) {
        historyTransitionEvent.forcedTransitionsInfo = std::make_shared<std::list<TransitionInfo>>();
        historyTransitionEvent.forcedTransitionsInfo->emplace_back(destinationState,
                                                                   historyParent,
                                                                   TransitionType::EXTERNAL_TRANSITION,
                                                                   nullptr,
                                                                   nullptr);
        historyTransitionEvent.ignoreEntryPoints = true;

        HSM_SYNC_EVENTS_QUEUE();
        mPendingEvents.push_front(historyTransitionEvent);
    }
}

void HierarchicalStateMachine::Impl::transitionToDefaultHistoryState(
    const StateID_t defaultTarget,
    const HsmTransitionCallback_t& defaultTargetTransitionCallback,
    const PendingEventInfo& event,
    const StateID_t destinationState) {
    std::list<StateID_t> historyTargets;
    StateID_t historyParent = INVALID_HSM_STATE_ID;

    if (true == getHistoryParent(destinationState, historyParent)) {
        HSM_TRACE_DEBUG("found parent=<%s> for history state=<%s>",
                        getStateName(historyParent).c_str(),
                        getStateName(destinationState).c_str());

        if (INVALID_HSM_STATE_ID == defaultTarget) {
            // transition to parent's entry point if there is no default history target
            historyTargets.push_back(historyParent);
        } else {
            historyTargets.push_back(defaultTarget);
            historyTargets.push_back(historyParent);
        }
    } else {
        HSM_TRACE_ERROR("parent for history state=<%s> wasnt found", getStateName(destinationState).c_str());
    }

    PendingEventInfo defHistoryTransitionEvent = event;

    defHistoryTransitionEvent.transitionType = TransitionBehavior::FORCED;

    for (const StateID_t historyTargetState : historyTargets) {
        HsmTransitionCallback_t cbTransition;

        defHistoryTransitionEvent.forcedTransitionsInfo = std::make_shared<std::list<TransitionInfo>>();

        if ((INVALID_HSM_STATE_ID != defaultTarget) && (historyTargetState == historyParent)) {
            defHistoryTransitionEvent.ignoreEntryPoints = true;
        } else {
            cbTransition = defaultTargetTransitionCallback;
        }

        defHistoryTransitionEvent.forcedTransitionsInfo->emplace_back(destinationState,
                                                                      historyTargetState,
                                                                      TransitionType::EXTERNAL_TRANSITION,
                                                                      cbTransition,
                                                                      nullptr);

        mPendingEvents.push_front(defHistoryTransitionEvent);
    }
}

bool HierarchicalStateMachine::Impl::processFinalStateTransition(const PendingEventInfo& event,
                                                                 const StateID_t destinationState) {
    const auto itFinalStateEvent = mFinalStates.find(destinationState);

    if (itFinalStateEvent != mFinalStates.end()) {
        StateID_t parentState = INVALID_HSM_STATE_ID;

        // don't generate events for top level final states since no one can process them
        if (true == getParentState(destinationState, parentState)) {
            // check if there are any other active siblings in this parent state
            // only generate final state event when all siblings got deactivated
            if (false == hasActiveChildren(parentState, false)) {
                PendingEventInfo finalStateEvent;

                finalStateEvent.transitionType = TransitionBehavior::REGULAR;
                finalStateEvent.args = event.args;

                if (INVALID_HSM_EVENT_ID != itFinalStateEvent->second) {
                    finalStateEvent.type = itFinalStateEvent->second;
                } else {
                    finalStateEvent.type = event.type;
                }

                {
                    HSM_SYNC_EVENTS_QUEUE();
                    mPendingEvents.push_front(finalStateEvent);
                }
            }
        }
    }

    return (itFinalStateEvent != mFinalStates.end());
}

HsmEventStatus HierarchicalStateMachine::Impl::handleSingleTransition(const StateID_t fromState,
                                                                                 const PendingEventInfo& event) {
    HSM_TRACE_CALL_DEBUG_ARGS("fromState=<%s>, event=<%s>, transitionType=%d",
                              getStateName(fromState).c_str(),
                              getEventName(event.type).c_str(),
                              SC2INT(event.transitionType));
    HsmEventStatus res = HsmEventStatus::DONE_FAILED;
    bool isCorrectTransition = false;
    std::list<TransitionInfo> matchingTransitions;

    DEBUG_DUMP_ACTIVE_STATES();

    // determine target state based on current transition
    isCorrectTransition = determineTargetState(event, fromState, matchingTransitions);

    // handle transition if it passed validation and has a target state
    if (true == isCorrectTransition) {
        bool isExitAllowed = true;
        std::list<StateID_t> exitedStates;

        // execute self transitions first
        if (true == executeSelfTransitions(event, matchingTransitions)) {
            res = HsmEventStatus::DONE_OK;
        }

        // execute exit transition (only once in case of parallel transitions)
        isExitAllowed = executeExitTransition(event, matchingTransitions, exitedStates);

        // proceed if transition was not blocked during state exit
        if (true == isExitAllowed) {
            for (const TransitionInfo& curTransition : matchingTransitions) {
                // everything except internal self-transitions
                if ((curTransition.fromState != curTransition.destinationState) ||
                    (TransitionType::EXTERNAL_TRANSITION == curTransition.transitionType)) {
                    res = processExternalTransition(event, fromState, curTransition, exitedStates);
                }
            }
        } else {
            res = HsmEventStatus::CANCELED;
        }
    }

    if (HsmEventStatus::DONE_FAILED == res) {
        HSM_TRACE_DEBUG("event <%s> in state <%s> was ignored.",
                        getEventName(event.type).c_str(),
                        getStateName(fromState).c_str());
    }

    DEBUG_DUMP_ACTIVE_STATES();
    HSM_TRACE_CALL_RESULT("%d", SC2INT(res));
    return res;
}

void HierarchicalStateMachine::Impl::clearPendingEvents() {
    HSM_TRACE_CALL_DEBUG_ARGS("clearPendingEvents: mPendingEvents.size()=%ld", mPendingEvents.size());

    for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()); ++it) {
        // since ongoing transitions can't be canceled we need to treat entry point transitions as atomic
        if (TransitionBehavior::REGULAR == it->transitionType) {
            it->releaseLock();
        }
    }

    mPendingEvents.clear();
}

bool HierarchicalStateMachine::Impl::hasSubstates(const StateID_t parent) const {
    return (mSubstates.find(parent) != mSubstates.end());
}

bool HierarchicalStateMachine::Impl::hasEntryPoint(const StateID_t state) const {
    return (mSubstateEntryPoints.find(state) != mSubstateEntryPoints.end());
}

bool HierarchicalStateMachine::Impl::getEntryPoints(const StateID_t state,
                                                    const EventID_t onEvent,
                                                    const VariantVector_t& transitionArgs,
                                                    std::list<StateID_t>& outEntryPoints) const {
    auto itRange = mSubstateEntryPoints.equal_range(state);

    outEntryPoints.clear();

    for (auto it = itRange.first; it != itRange.second; ++it) {
        if (((INVALID_HSM_EVENT_ID == it->second.onEvent) || (onEvent == it->second.onEvent)) &&
            // check transition condition if it was defined
            ((nullptr == it->second.checkCondition) ||
             (it->second.checkCondition(transitionArgs) == it->second.expectedConditionValue))) {
            outEntryPoints.push_back(it->second.state);
        }
    }

    return (false == outEntryPoints.empty());
}

bool HierarchicalStateMachine::Impl::replaceActiveState(const StateID_t oldState, const StateID_t newState) {
    HSM_TRACE_CALL_DEBUG_ARGS("oldState=<%s>, newState=<%s>", getStateName(oldState).c_str(), getStateName(newState).c_str());

    if (false == isSubstateOf(oldState, newState)) {
        mActiveStates.remove(oldState);
    }

    return addActiveState(newState);
}

bool HierarchicalStateMachine::Impl::addActiveState(const StateID_t newState) {
    HSM_TRACE_CALL_DEBUG_ARGS("newState=<%s>", getStateName(newState).c_str());
    bool wasAdded = false;

    if (false == isStateActive(newState)) {
        mActiveStates.push_back(newState);
        wasAdded = true;
    }

    HSM_TRACE_DEBUG("mActiveStates.size=%d", SC2INT(mActiveStates.size()));
    return wasAdded;
}

#ifdef HSM_ENABLE_SAFE_STRUCTURE

bool HierarchicalStateMachine::Impl::isTopState(const StateID_t state) const {
    auto it = std::find(mTopLevelStates.begin(), mTopLevelStates.end(), state);

    return (it == mTopLevelStates.end());
}

bool HierarchicalStateMachine::Impl::isSubstate(const StateID_t state) const {
    bool result = false;

    for (const auto& curSubstate : mSubstates) {
        if (curSubstate.second == state) {
            result = true;
            break;
        }
    }

    return result;
}

bool HierarchicalStateMachine::Impl::hasParentState(const StateID_t state, StateID_t& outParent) const {
    bool hasParent = false;

    for (const auto& curSubstate : mSubstates) {
        if (state == curSubstate.second) {
            hasParent = true;
            outParent = curSubstate.first;  // cppcheck-suppress misra-c2012-17.8 ; outParent is used to return result
            break;
        }
    }

    return hasParent;
}

#endif  // HSM_ENABLE_SAFE_STRUCTURE

bool HierarchicalStateMachine::Impl::enableHsmDebugging() {
#ifdef HSMBUILD_DEBUGGING
    constexpr const char* DEFAULT_DUMP_PATH = "./dump.hsmlog";
    constexpr const char* ENV_DUMPPATH = "HSMCPP_DUMP_PATH";
    // NOLINTNEXTLINE(concurrency-mt-unsafe): enableHsmDebugging() doesn't need to be thread-safe
    char* envPath = std::getenv(ENV_DUMPPATH);

    return enableHsmDebugging((nullptr == envPath) ? DEFAULT_DUMP_PATH : std::string(envPath));
#else
    return true;
#endif
}

bool HierarchicalStateMachine::Impl::enableHsmDebugging(const std::string& dumpPath) {
#ifdef HSMBUILD_DEBUGGING
    bool res = false;
    bool isNewLog = (access(dumpPath.c_str(), F_OK) != 0);

    if (nullptr != mHsmLogFile.open(dumpPath.c_str(), std::ios::out | std::ios::app)) {
        mHsmLog = std::make_shared<std::ostream>(&mHsmLogFile);

        if (true == isNewLog) {
            *mHsmLog << "---\n";
            mHsmLog->flush();
        }

        res = true;
    }

    return res;
#else
    return true;
#endif
}

void HierarchicalStateMachine::Impl::disableHsmDebugging() {
#ifdef HSMBUILD_DEBUGGING
    mHsmLogFile.close();
#endif
}

void HierarchicalStateMachine::Impl::logHsmAction(const HsmLogAction action,
                                                  const StateID_t fromState,
                                                  const StateID_t targetState,
                                                  const EventID_t event,
                                                  const bool hasFailed,
                                                  const VariantVector_t& args) {
#ifdef HSMBUILD_DEBUGGING
    if (true == mHsmLogFile.is_open()) {
        static const std::map<HsmLogAction, std::string> actionsMap = {
            std::make_pair(HsmLogAction::IDLE, "idle"),
            std::make_pair(HsmLogAction::TRANSITION, "transition"),
            std::make_pair(HsmLogAction::TRANSITION_ENTRYPOINT, "transition_entrypoint"),
            std::make_pair(HsmLogAction::CALLBACK_EXIT, "callback_exit"),
            std::make_pair(HsmLogAction::CALLBACK_ENTER, "callback_enter"),
            std::make_pair(HsmLogAction::CALLBACK_STATE, "callback_state"),
            std::make_pair(HsmLogAction::ON_ENTER_ACTIONS, "onenter_actions"),
            std::make_pair(HsmLogAction::ON_EXIT_ACTIONS, "onexit_actions")};
        constexpr size_t bufTimeSize = 80;
        constexpr size_t bufTimeMsSize = 6;
        std::array<char, bufTimeSize> bufTime = {0};
        std::array<char, bufTimeMsSize> bufTimeMs = {0};
        auto currentTimePoint = std::chrono::system_clock::now();
        const std::time_t tt = std::chrono::system_clock::to_time_t(currentTimePoint);
        std::tm timeinfo = {0};
        const std::tm* tmResult = nullptr;  // this is just to check that localtime was executed correctly

  #ifdef WIN32
        if (0 == ::localtime_s(&timeinfo, &tt)) {
            tmResult = &timeinfo;
        }
  #else
        // TODO: function is not thread safe [concurrency-mt-unsafe]
        tmResult = localtime(&tt);
        if (nullptr != tmResult) {
            timeinfo = *tmResult;
        }
  #endif  // WIN32

        if (nullptr != tmResult) {
            constexpr int millisecondsPerSecond = 1000;

            (void)std::strftime(bufTime.data(), bufTime.size(), "%Y-%m-%d %H:%M:%S", &timeinfo);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): using snprintf instead stringstream for performance reasons
            (void)snprintf(
                bufTimeMs.data(),
                bufTimeMs.size(),
                ".%03d",
                static_cast<int>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint.time_since_epoch()).count() %
                    millisecondsPerSecond));
        } else {
            (void)std::strncpy(bufTime.data(), "0000-00-00 00:00:00", bufTime.size() - 1U);
            (void)std::strncpy(bufTimeMs.data(), ".000", bufTimeMs.size() - 1U);
        }

        *mHsmLog << "\n-\n"
                    "  timestamp: \""
                 << bufTime.data() << bufTimeMs.data()
                 << "\"\n"
                    "  active_states:";

        for (const auto& curState : mActiveStates) {
            *mHsmLog << "\n    - \"" << getStateName(curState) << "\"";
        }

        *mHsmLog << "\n  action: " << actionsMap.at(action)
                 << "\n"
                    "  from_state: \""
                 << getStateName(fromState)
                 << "\"\n"
                    "  target_state: \""
                 << getStateName(targetState)
                 << "\"\n"
                    "  event: \""
                 << getEventName(event)
                 << "\"\n"
                    "  status: "
                 << (hasFailed ? "failed" : "")
                 << "\n"
                    "  args:";

        for (const auto& curArg : args) {
            *mHsmLog << "\n    - " << curArg.toString();
        }

        mHsmLog->flush();
    }
#endif  // HSMBUILD_DEBUGGING
}

#ifndef HSM_DISABLE_DEBUG_TRACES
void HierarchicalStateMachine::Impl::dumpActiveStates() {
    HSM_TRACE_CALL();

    std::string temp;

    for (const auto& curState : mActiveStates) {
        temp += getStateName(curState) + std::string(", ");
    }

    HSM_TRACE_DEBUG("active states: <%s>", temp.c_str());
}

std::string HierarchicalStateMachine::Impl::getStateName(const StateID_t state) {
    std::string res;
  #ifndef HSM_DISABLE_THREADSAFETY
    LockGuard lk(mParentSync);
  #endif

    if (nullptr != mParent) {
        res = mParent->getStateName(state);
    }

    return res;
}

std::string HierarchicalStateMachine::Impl::getEventName(const EventID_t event) {
    std::string res;
  #ifndef HSM_DISABLE_THREADSAFETY
    LockGuard lk(mParentSync);
  #endif

    if (nullptr != mParent) {
        res = mParent->getEventName(event);
    }

    return res;
}
#else   // HSM_DISABLE_DEBUG_TRACES

std::string HierarchicalStateMachine::Impl::getStateName(const StateID_t state) {
    return std::string();
}

std::string HierarchicalStateMachine::Impl::getEventName(const EventID_t event) {
    return std::string();
}
#endif  // HSM_DISABLE_DEBUG_TRACES

}  // namespace hsmcpp