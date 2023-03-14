// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "HsmImpl.hpp"

#include <chrono>
#include <algorithm>
#include <ctime>

#include "hsmcpp/IHsmEventDispatcher.hpp"
#include "hsmcpp/logging.hpp"

#if !defined(HSM_DISABLE_THREADSAFETY) && defined(FREERTOS_AVAILABLE)
  #include "hsmcpp/os/InterruptsFreeSection.hpp"
#endif

#ifdef HSMBUILD_DEBUGGING
  #include <cstdlib>
  #include <cstring>

// WIN, access
  #ifdef WIN32
    #include <io.h>
    #define F_OK 0
  #else
    #include <unistd.h>
        // cppcheck-suppress misra-c2012-21.10
    #include <time.h>
  #endif
#endif

#ifndef HSM_DISABLE_DEBUG_TRACES
  #define DEBUG_DUMP_ACTIVE_STATES() dumpActiveStates()
#else
  #define DEBUG_DUMP_ACTIVE_STATES()
#endif

// NOTE: used only for logging during development. in release mode macros is empty
// cppcheck-suppress misra-c2012-8.4
HSM_TRACE_PREINIT()

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HierarchicalStateMachine"

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

#define ENV_DUMPPATH "HSMCPP_DUMP_PATH"
#define DEFAULT_DUMP_PATH "./dump.hsmlog"

#define HsmEventStatus_t HierarchicalStateMachine::Impl::HsmEventStatus

namespace hsmcpp {

// ============================================================================
// PUBLIC
// ============================================================================
HierarchicalStateMachine::Impl::Impl(HierarchicalStateMachine& parent, const StateID_t initialState)
    : mParent(parent)
    , mInitialState(initialState) {
    HSM_TRACE_INIT();
}

HierarchicalStateMachine::Impl::~Impl() {
    release();
}

void HierarchicalStateMachine::Impl::setInitialState(const StateID_t initialState) {
    if (!mDispatcher) {
        mInitialState = initialState;
    }
}

bool HierarchicalStateMachine::Impl::initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher) {
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    if (!mDispatcher) {
        // NOTE: false-positive. std::shated_ptr has a bool() operator
        // cppcheck-suppress misra-c2012-14.4
        if (dispatcher) {
            if (true == dispatcher->start()) {
                mDispatcher = dispatcher;
                mEventsHandlerId =
                    mDispatcher->registerEventHandler(std::bind(&HierarchicalStateMachine::Impl::dispatchEvents, this));
                mTimerHandlerId = mDispatcher->registerTimerHandler(
                    std::bind(&HierarchicalStateMachine::Impl::dispatchTimerEvent, this, std::placeholders::_1));
                mEnqueuedEventsHandlerId =
                    mDispatcher->registerEnqueuedEventHandler([&](const EventID_t event) { transitionSimple(event); });

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
                    mDispatcher->unregisterEventHandler(mEventsHandlerId);
                    mDispatcher->unregisterEnqueuedEventHandler(mEnqueuedEventsHandlerId);
                    mDispatcher->unregisterTimerHandler(mTimerHandlerId);
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
    return (nullptr != mDispatcher);
}

void HierarchicalStateMachine::Impl::release() {
    mStopDispatching = true;
    HSM_TRACE_CALL_DEBUG();

    disableHsmDebugging();

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shated_ptr has a bool() operator
    if (mDispatcher) {
        mDispatcher->unregisterEventHandler(mEventsHandlerId);
        mDispatcher->unregisterEnqueuedEventHandler(mEnqueuedEventsHandlerId);
        mDispatcher->unregisterTimerHandler(mTimerHandlerId);
        mDispatcher.reset();
        mEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    }
}

void HierarchicalStateMachine::Impl::registerFailedTransitionCallback(const HsmTransitionFailedCallback_t& onFailedTransition) {
    mFailedTransitionCallback = onFailedTransition;
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
        StateCallbacks cb;

        cb.onStateChanged = onStateChanged;
        cb.onEntering = onEntering;
        cb.onExiting = onExiting;
        mRegisteredStates[state] = cb;

        HSM_TRACE_CALL_DEBUG_ARGS("mRegisteredStates.size=%ld", mRegisteredStates.size());
    }
}

void HierarchicalStateMachine::Impl::registerFinalState(const StateID_t state,
                                                        const EventID_t event,
                                                        HsmStateChangedCallback_t onStateChanged,
                                                        HsmStateEnterCallback_t onEntering,
                                                        HsmStateExitCallback_t onExiting) {
    mFinalStates.emplace(state, event);
    registerState(state, onStateChanged, onEntering, onExiting);
}

void HierarchicalStateMachine::Impl::registerHistory(const StateID_t parent,
                                                     const StateID_t historyState,
                                                     const HistoryType type,
                                                     const StateID_t defaultTarget,
                                                     HsmTransitionCallback_t transitionCallback) {
    (void)mHistoryStates.emplace(parent, historyState);
    mHistoryData.emplace(historyState, HistoryInfo(type, defaultTarget, transitionCallback));
}

bool HierarchicalStateMachine::Impl::registerSubstate(const StateID_t parent, const StateID_t substate) {
    return registerSubstate(parent, substate, false);
}

bool HierarchicalStateMachine::Impl::registerSubstateEntryPoint(const StateID_t parent,
                                                                const StateID_t substate,
                                                                const EventID_t onEvent,
                                                                const HsmTransitionConditionCallback_t& conditionCallback,
                                                                const bool expectedConditionValue) {
    return registerSubstate(parent, substate, true, onEvent, conditionCallback, expectedConditionValue);
}

void HierarchicalStateMachine::Impl::registerTimer(const TimerID_t timerID, const EventID_t event) {
    mTimers.emplace(timerID, event);
}

bool HierarchicalStateMachine::Impl::registerSubstate(const StateID_t parent,
                                                      const StateID_t substate,
                                                      const bool isEntryPoint,
                                                      const EventID_t onEvent,
                                                      const HsmTransitionConditionCallback_t& conditionCallback,
                                                      const bool expectedConditionValue) {
    bool registrationAllowed = false;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    // do a simple sanity check
    if (parent != substate) {
        StateID_t curState = parent;
        StateID_t prevState;

        if (false == hasParentState(substate, prevState)) {
            registrationAllowed = true;

            while (true == hasParentState(curState, prevState)) {
                if (substate == prevState) {
                    HSM_TRACE_CALL_DEBUG_ARGS(
                        "requested operation will result in substates recursion (parent=<%s>, substate=<%s>)",
                        mParent.getStateName(parent).c_str(),
                        mParent.getStateName(substate).c_str());
                    registrationAllowed = false;
                    break;
                }

                curState = prevState;
            }
        } else {
            HSM_TRACE_CALL_DEBUG_ARGS("substate <%s> already has a parent <%s>",
                                      mParent.getStateName(substate).c_str(),
                                      mParent.getStateName(prevState).c_str());
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
            entryInfo.onEvent = onEvent;
            entryInfo.checkCondition = conditionCallback;
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
                              mParent.getStateName(state).c_str(),
                              SC2INT(actionTrigger),
                              SC2INT(action));
    bool result = false;
    bool argsValid = false;
    StateActionInfo newAction;

    newAction.actionArgs = args;

    // validate arguments
    switch (action) {
        case StateAction::START_TIMER:
            argsValid = (newAction.actionArgs.size() == 3u) && newAction.actionArgs[0].isNumeric() &&
                        newAction.actionArgs[1].isNumeric() && newAction.actionArgs[2].isBool();
            break;
        case StateAction::RESTART_TIMER:
        case StateAction::STOP_TIMER:
            argsValid = (newAction.actionArgs.size() == 1u) && newAction.actionArgs[0].isNumeric();
            break;
        case StateAction::TRANSITION:
            argsValid = (newAction.actionArgs.size() >= 1u) && newAction.actionArgs[0].isNumeric();
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

void HierarchicalStateMachine::Impl::registerTransition(const StateID_t from,
                                                        const StateID_t to,
                                                        const EventID_t onEvent,
                                                        HsmTransitionCallback_t transitionCallback,
                                                        HsmTransitionConditionCallback_t conditionCallback,
                                                        const bool expectedConditionValue) {
    (void)mTransitionsByEvent.emplace(std::make_pair(from, onEvent),
                                      TransitionInfo(from,
                                                     to,
                                                     TransitionType::EXTERNAL_TRANSITION,
                                                     transitionCallback,
                                                     conditionCallback,
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
        TransitionInfo(state, state, type, transitionCallback, conditionCallback, expectedConditionValue));
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
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, args.size=%lu", mParent.getEventName(event).c_str(), args.size());

    (void)transitionExWithArgsArray(event, false, false, 0, args);
}

bool HierarchicalStateMachine::Impl::transitionExWithArgsArray(const EventID_t event,
                                                               const bool clearQueue,
                                                               const bool sync,
                                                               const int timeoutMs,
                                                               const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, clearQueue=%s, sync=%s, args.size=%lu",
                              mParent.getEventName(event).c_str(),
                              BOOL2STR(clearQueue),
                              BOOL2STR(sync),
                              args.size());

    bool status = false;

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shated_ptr has a bool() operator
    if (mDispatcher) {
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
        mDispatcher->emitEvent(mEventsHandlerId);

        if (true == sync) {
            HSM_TRACE_DEBUG("transitionEx: wait...");
            eventInfo.wait(timeoutMs);
            status = (HsmEventStatus_t::DONE_OK == *eventInfo.transitionStatus);
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

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shated_ptr has a bool() operator
    if (mDispatcher) {
        res = mDispatcher->enqueueEvent(mEnqueuedEventsHandlerId, event);
    }

    return res;
}

bool HierarchicalStateMachine::Impl::isTransitionPossible(const EventID_t event, const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", mParent.getEventName(event).c_str());
    bool possible = false;

    for (auto it = mActiveStates.begin(); it != mActiveStates.end(); ++it) {
        possible = checkTransitionPossibility(*it, event, args);

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
    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->startTimer(mTimerHandlerId, timerID, intervalMs, isSingleShot);
    }
}

void HierarchicalStateMachine::Impl::restartTimer(const TimerID_t timerID) {
    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->restartTimer(timerID);
    }
}

void HierarchicalStateMachine::Impl::stopTimer(const TimerID_t timerID) {
    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->stopTimer(timerID);
    }
}

// ============================================================================
// PRIVATE
// ============================================================================
void HierarchicalStateMachine::Impl::handleStartup() {
    HSM_TRACE_CALL_DEBUG_ARGS("mActiveStates.size=%ld", mActiveStates.size());

    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        HSM_TRACE_DEBUG("state=<%s>", mParent.getStateName(mInitialState).c_str());
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
            mDispatcher->emitEvent(mEventsHandlerId);
        }
    }
}

void HierarchicalStateMachine::Impl::transitionSimple(const EventID_t event) {
    (void)transitionExWithArgsArray(event, false, false, 0, VariantVector_t());
}

void HierarchicalStateMachine::Impl::dispatchEvents() {
    HSM_TRACE_CALL_DEBUG_ARGS("mPendingEvents.size=%ld", mPendingEvents.size());

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shated_ptr has a bool() operator
    if (mDispatcher) {
        if (false == mStopDispatching) {
            if (false == mPendingEvents.empty()) {
                PendingEventInfo pendingEvent;

                {
                    HSM_SYNC_EVENTS_QUEUE();
                    pendingEvent = mPendingEvents.front();
                    mPendingEvents.pop_front();
                }

                HsmEventStatus_t transitiontStatus = doTransition(pendingEvent);

                HSM_TRACE_DEBUG("unlock with status %d", SC2INT(transitiontStatus));
                pendingEvent.unlock(transitiontStatus);
            }

            if ((false == mStopDispatching) && (false == mPendingEvents.empty())) {
                mDispatcher->emitEvent(mEventsHandlerId);
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
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", mParent.getStateName(state).c_str());
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
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", mParent.getStateName(state).c_str());
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
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", mParent.getStateName(state).c_str());
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onStateChanged) {
        it->second.onStateChanged(args);
        logHsmAction(HsmLogAction::CALLBACK_STATE, INVALID_HSM_STATE_ID, state, INVALID_HSM_EVENT_ID, false, args);
    } else {
        HSM_TRACE_WARNING("no callback registered for state <%s>", mParent.getStateName(state).c_str());
    }
}

void HierarchicalStateMachine::Impl::executeStateAction(const StateID_t state, const StateActionTrigger actionTrigger) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>, actionTrigger=%d", mParent.getStateName(state).c_str(), SC2INT(actionTrigger));

    // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shated_ptr has a bool() operator
    if (mDispatcher) {
        auto key = std::make_pair(state, actionTrigger);
        auto itRange = mRegisteredActions.equal_range(key);

        if (itRange.first != itRange.second) {
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
                    mDispatcher->startTimer(mTimerHandlerId,
                                            actionInfo.actionArgs[0].toInt64(),
                                            actionInfo.actionArgs[1].toInt64(),
                                            actionInfo.actionArgs[2].toBool());
                } else if (StateAction::STOP_TIMER == actionInfo.action) {
                    mDispatcher->stopTimer(actionInfo.actionArgs[0].toInt64());
                } else if (StateAction::RESTART_TIMER == actionInfo.action) {
                    mDispatcher->restartTimer(actionInfo.actionArgs[0].toInt64());
                } else if (StateAction::TRANSITION == actionInfo.action) {
                    VariantVector_t transitionArgs;

                    if (actionInfo.actionArgs.size() > 1u) {
                        transitionArgs.reserve(actionInfo.actionArgs.size() - 1u);

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
}

bool HierarchicalStateMachine::Impl::getParentState(const StateID_t child, StateID_t& outParent) {
    bool wasFound = false;
    auto it = std::find_if(mSubstates.begin(), mSubstates.end(), [child](const std::pair<StateID_t, StateID_t>& item) {
        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return (child == item.second);
    });

    if (mSubstates.end() != it) {
        outParent = it->first;  // cppcheck-suppress misra-c2012-17.8 ; outParent is used to return result
        wasFound = true;
    }

    return wasFound;
}
bool HierarchicalStateMachine::Impl::isSubstateOf(const StateID_t parent, const StateID_t child) {
    HSM_TRACE_CALL_DEBUG_ARGS("parent=<%s>, child=<%s>",
                              mParent.getStateName(parent).c_str(),
                              mParent.getStateName(child).c_str());
    StateID_t curState = child;

    do {
        if (false == getParentState(curState, curState)) {
            break;
        }
    } while (parent != curState);

    return (parent == curState);
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
                              mParent.getStateName(topLevelState).c_str(),
                              exitedStates.size());

    std::list<std::list<StateID_t>*> upatedHistory;

    for (auto itActiveState = exitedStates.begin(); itActiveState != exitedStates.end(); ++itActiveState) {
        StateID_t curState = *itActiveState;
        StateID_t parentState;

        while (true == getParentState(curState, parentState)) {
            HSM_TRACE_DEBUG("curState=<%s>, parentState=<%s>",
                            mParent.getStateName(curState).c_str(),
                            mParent.getStateName(parentState).c_str());
            auto itRange = mHistoryStates.equal_range(parentState);

            if (itRange.first != itRange.second) {
                HSM_TRACE_DEBUG("parent=<%s> has history items", mParent.getStateName(parentState).c_str());

                for (auto it = itRange.first; it != itRange.second; ++it) {
                    auto itCurHistory = mHistoryData.find(it->second);

                    if (itCurHistory != mHistoryData.end()) {
                        auto itUpdatedHistory =
                            std::find(upatedHistory.begin(), upatedHistory.end(), &itCurHistory->second.previousActiveStates);

                        if (itUpdatedHistory == upatedHistory.end()) {
                            itCurHistory->second.previousActiveStates.clear();
                            upatedHistory.push_back(&(itCurHistory->second.previousActiveStates));
                        } else {
                        }

                        if (HistoryType::SHALLOW == itCurHistory->second.type) {
                            if (std::find(itCurHistory->second.previousActiveStates.begin(),
                                          itCurHistory->second.previousActiveStates.end(),
                                          curState) == itCurHistory->second.previousActiveStates.end()) {
                                HSM_TRACE_DEBUG("SHALLOW -> store state <%s> in history of parent <%s>",
                                                mParent.getStateName(curState).c_str(),
                                                mParent.getStateName(it->second).c_str());
                                itCurHistory->second.previousActiveStates.push_back(curState);
                            }
                        } else if (HistoryType::DEEP == itCurHistory->second.type) {
                            if (std::find(itCurHistory->second.previousActiveStates.begin(),
                                          itCurHistory->second.previousActiveStates.end(),
                                          *itActiveState) == itCurHistory->second.previousActiveStates.end()) {
                                HSM_TRACE_DEBUG("DEEP -> store state <%s> in history of parent <%s>",
                                                mParent.getStateName(*itActiveState).c_str(),
                                                mParent.getStateName(it->second).c_str());
                                itCurHistory->second.previousActiveStates.push_back(*itActiveState);
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
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", mParent.getEventName(event).c_str());

    StateID_t currentState = fromState;
    std::list<TransitionInfo> possibleTransitions;
    EventID_t nextEvent;
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
    HSM_TRACE_CALL_DEBUG_ARGS("fromState=<%s>, event=<%s>",
                              mParent.getStateName(fromState).c_str(),
                              mParent.getEventName(event).c_str());
    bool continueSearch;
    StateID_t curState = fromState;

    do {
        auto key = std::make_pair(curState, event);
        auto itRange = mTransitionsByEvent.equal_range(key);

        continueSearch = false;

        if (itRange.first == itRange.second) {
            if (true == searchParents) {
                StateID_t parentState;
                bool hasParent = getParentState(curState, parentState);

                if (true == hasParent) {
                    curState = parentState;
                    continueSearch = true;
                }
            }
        } else {
            for (auto it = itRange.first; it != itRange.second; ++it) {
                HSM_TRACE_DEBUG("check transition to <%s>...", mParent.getStateName(it->second.destinationState).c_str());

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
                                HSM_TRACE_DEBUG("state <%s> has entrypoints", mParent.getStateName(currentParent).c_str());
                                std::list<StateID_t> entryPoints;

                                if (true == getEntryPoints(currentParent, event, transitionArgs, entryPoints)) {
                                    parentStates.splice(parentStates.end(), entryPoints);
                                } else {
                                    HSM_TRACE_WARNING("no matching entrypoints found");
                                    break;
                                }
                            } else {
                                HSM_TRACE_WARNING("state <%s> doesn't have an entrypoint defined",
                                                  mParent.getStateName(currentParent).c_str());
                                break;
                            }
                        } else {
                            outTransitions.push_back(it->second);
                            wasFound = true;
                        }
                    } while ((false == wasFound) && (parentStates.empty() == false));
                }
            }
        }
    } while (true == continueSearch);

    HSM_TRACE_CALL_RESULT("%s", BOOL2STR(outTransitions.empty() == false));
    return (outTransitions.empty() == false);
}

typename HsmEventStatus_t HierarchicalStateMachine::Impl::doTransition(const PendingEventInfo& event) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, transitionType=%d",
                              mParent.getEventName(event.type).c_str(),
                              SC2INT(event.transitionType));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
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
                const HsmEventStatus_t singleTransitionResult = handleSingleTransition(*it, event);

                switch (singleTransitionResult) {
                    case HsmEventStatus_t::PENDING:
                        res = singleTransitionResult;
                        acceptedStates.push_back(*it);
                        break;
                    case HsmEventStatus_t::DONE_OK:
                        logHsmAction(HsmLogAction::IDLE,
                                     INVALID_HSM_STATE_ID,
                                     INVALID_HSM_STATE_ID,
                                     INVALID_HSM_EVENT_ID,
                                     false,
                                     VariantVector_t());
                        if (HsmEventStatus_t::PENDING != res) {
                            res = singleTransitionResult;
                        }
                        acceptedStates.push_back(*it);
                        break;
                    case HsmEventStatus_t::CANCELED:
                    case HsmEventStatus_t::DONE_FAILED:
                    default:
                        // do nothing
                        break;
                }
            }
        }
    }

    if (mFailedTransitionCallback && ((HsmEventStatus_t::DONE_FAILED == res) || (HsmEventStatus_t::CANCELED == res))) {
        mFailedTransitionCallback(event.type, event.args);
    }

    HSM_TRACE_CALL_RESULT("%d", SC2INT(res));
    return res;
}

typename HsmEventStatus_t HierarchicalStateMachine::Impl::handleSingleTransition(const StateID_t activeState,
                                                                                 const PendingEventInfo& event) {
    HSM_TRACE_CALL_DEBUG_ARGS("activeState=<%s>, event=<%s>, transitionType=%d",
                              mParent.getStateName(activeState).c_str(),
                              mParent.getEventName(event.type).c_str(),
                              SC2INT(event.transitionType));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
    const StateID_t fromState = activeState;
    bool isCorrectTransition = false;
    std::list<TransitionInfo> matchingTransitions;

    DEBUG_DUMP_ACTIVE_STATES();

    // ========================================================
    // determine target state based on current transition
    if (TransitionBehavior::REGULAR == event.transitionType) {
        isCorrectTransition = findTransitionTarget(fromState, event.type, event.args, false, matchingTransitions);

        if (false == isCorrectTransition) {
            HSM_TRACE_WARNING("no suitable transition from state <%s> with event <%s>",
                              mParent.getStateName(fromState).c_str(),
                              mParent.getEventName(event.type).c_str());
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
                for (auto it = entryStates.begin(); it != entryStates.end(); ++it) {
                    (void)matchingTransitions.emplace_back(
                        TransitionInfo{fromState, *it, TransitionType::EXTERNAL_TRANSITION, nullptr, nullptr});
                }
            } else {
                HSM_TRACE_WARNING("state <%s> doesn't have a suitable entry point (event <%s>)",
                                  mParent.getStateName(fromState).c_str(),
                                  mParent.getEventName(event.type).c_str());
            }
        }
    } else if (TransitionBehavior::FORCED == event.transitionType) {
        HSM_TRACE_DEBUG("forced history transitions: %d", SC2INT(event.forcedTransitionsInfo->size()));
        matchingTransitions = *event.forcedTransitionsInfo;
        isCorrectTransition = true;
    } else {
        // NOTE: do nothing
    }

    // ========================================================
    // handle transition if it passed validation and has a target state
    if (true == isCorrectTransition) {
        bool isExitAllowed = true;
        std::list<StateID_t> exitedStates;

        // execute self transitions first
        for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it) {
            if ((it->fromState == it->destinationState) && (TransitionType::INTERNAL_TRANSITION == it->transitionType)) {
                // TODO: separate type for self transition?
                logHsmAction(HsmLogAction::TRANSITION, it->fromState, it->destinationState, event.type, false, event.args);

                // NOTE: false-positive. std::function has a bool() operator
                // cppcheck-suppress misra-c2012-14.4
                if (it->onTransition) {
                    it->onTransition(event.args);
                }

                res = HsmEventStatus_t::DONE_OK;
            }
        }

        // execute exit transition (only once in case of parallel transitions)
        for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it) {
            // everything except internal self-transitions
            if ((it->fromState != it->destinationState) || (TransitionType::EXTERNAL_TRANSITION == it->transitionType)) {
                // exit active states only during regular transitions
                if (TransitionBehavior::REGULAR == event.transitionType) {
                    // it's an outer transition from parent state. we need to find and exit all active substates
                    for (auto itActiveState = mActiveStates.rbegin(); itActiveState != mActiveStates.rend(); ++itActiveState) {
                        HSM_TRACE_DEBUG("OUTER EXIT: FROM=%s, ACTIVE=%s",
                                        mParent.getStateName(it->fromState).c_str(),
                                        mParent.getStateName(*itActiveState).c_str());
                        if ((it->fromState == *itActiveState) || (true == isSubstateOf(it->fromState, *itActiveState))) {
                            isExitAllowed = onStateExiting(*itActiveState);

                            if (true == isExitAllowed) {
                                exitedStates.push_back(*itActiveState);
                            } else {
                                break;
                            }
                        }
                    }

                    // if no one blocked ongoing transition - remove child states from active list
                    if (true == isExitAllowed) {
                        // store history for states between "fromState" ----> "it->fromState"
                        updateHistory(it->fromState, exitedStates);

                        for (auto itState = exitedStates.begin(); itState != exitedStates.end(); ++itState) {
                            mActiveStates.remove(*itState);
                        }
                    }
                    // if one of the states blocked ongoing transition we need to rollback
                    else {
                        for (auto itState = exitedStates.begin(); itState != exitedStates.end(); ++itState) {
                            mActiveStates.remove(*itState);
                            // to prevent infinite loops we don't allow state to cancel transition
                            (void)onStateEntering(*itState, VariantVector_t());
                            mActiveStates.push_back(*itState);
                            onStateChanged(*itState, VariantVector_t());
                        }
                    }
                }
            }
        }

        // proceed if transition was not blocked during state exit
        if (true == isExitAllowed) {
            for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it) {
                // everything except internal self-transitions
                if ((it->fromState != it->destinationState) || (TransitionType::EXTERNAL_TRANSITION == it->transitionType)) {
                    // NOTE: Decide if we need functionality to cancel ongoing transition
                    logHsmAction(
                        ((TransitionBehavior::ENTRYPOINT != event.transitionType) ? HsmLogAction::TRANSITION
                                                                                  : HsmLogAction::TRANSITION_ENTRYPOINT),
                        it->fromState,
                        it->destinationState,
                        event.type,
                        false,
                        event.args);

                    // NOTE: false-positive. std::shated_ptr has a bool() operator
                    // cppcheck-suppress misra-c2012-14.4
                    if (it->onTransition) {
                        it->onTransition(event.args);
                    }

                    if (true == onStateEntering(it->destinationState, event.args)) {
                        std::list<StateID_t> entryPoints;

                        if (true == replaceActiveState(fromState, it->destinationState)) {
                            onStateChanged(it->destinationState, event.args);
                        }

                        // check if current state is a final state
                        const auto itFinalStateEvent = mFinalStates.find(it->destinationState);

                        if (itFinalStateEvent != mFinalStates.end()) {
                            StateID_t parentState = INVALID_HSM_STATE_ID;

                            // don't generate events for top level final states since no one can process them
                            if (true == getParentState(it->destinationState, parentState)) {
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

                            res = HsmEventStatus_t::DONE_OK;
                        } else {
                            // check if we transitioned into history state
                            auto itHistoryData = mHistoryData.find(it->destinationState);

                            if (itHistoryData != mHistoryData.end()) {
                                HSM_TRACE_DEBUG("state=<%s> is a history state with %ld stored states",
                                                mParent.getStateName(it->destinationState).c_str(),
                                                itHistoryData->second.previousActiveStates.size());

                                // transition to previous states
                                if (itHistoryData->second.previousActiveStates.empty() == false) {
                                    PendingEventInfo historyTransitionEvent = event;

                                    historyTransitionEvent.transitionType = TransitionBehavior::FORCED;
                                    historyTransitionEvent.forcedTransitionsInfo =
                                        std::make_shared<std::list<TransitionInfo>>();

                                    auto itPrevChildState = itHistoryData->second.previousActiveStates.end();

                                    {
                                        HSM_SYNC_EVENTS_QUEUE();

                                        for (auto itPrevState = itHistoryData->second.previousActiveStates.begin();
                                             itPrevState != itHistoryData->second.previousActiveStates.end();
                                             ++itPrevState) {
                                            if ((itPrevChildState != itHistoryData->second.previousActiveStates.end()) &&
                                                (true == isSubstateOf(*itPrevState, *itPrevChildState))) {
                                                if (false == historyTransitionEvent.forcedTransitionsInfo->empty()) {
                                                    mPendingEvents.push_front(historyTransitionEvent);
                                                }

                                                historyTransitionEvent.forcedTransitionsInfo =
                                                    std::make_shared<std::list<TransitionInfo>>();
                                                historyTransitionEvent.ignoreEntryPoints = true;
                                            } else {
                                                historyTransitionEvent.ignoreEntryPoints = false;
                                            }

                                            itPrevChildState = itPrevState;
                                            historyTransitionEvent.forcedTransitionsInfo->emplace_back(
                                                it->destinationState,
                                                *itPrevState,
                                                TransitionType::EXTERNAL_TRANSITION,
                                                nullptr,
                                                nullptr);
                                        }

                                        mPendingEvents.push_front(historyTransitionEvent);
                                    }

                                    itHistoryData->second.previousActiveStates.clear();

                                    StateID_t historyParent;

                                    if (true == getHistoryParent(it->destinationState, historyParent)) {
                                        historyTransitionEvent.forcedTransitionsInfo =
                                            std::make_shared<std::list<TransitionInfo>>();
                                        historyTransitionEvent.forcedTransitionsInfo->emplace_back(
                                            it->destinationState,
                                            historyParent,
                                            TransitionType::EXTERNAL_TRANSITION,
                                            nullptr,
                                            nullptr);
                                        historyTransitionEvent.ignoreEntryPoints = true;

                                        HSM_SYNC_EVENTS_QUEUE();
                                        mPendingEvents.push_front(historyTransitionEvent);
                                    }
                                }
                                // transition to default state or entry point
                                else {
                                    std::list<StateID_t> historyTargets;
                                    StateID_t historyParent;

                                    if (true == getHistoryParent(it->destinationState, historyParent)) {
                                        HSM_TRACE_DEBUG("found parent=<%s> for history state=<%s>",
                                                        mParent.getStateName(historyParent).c_str(),
                                                        mParent.getStateName(it->destinationState).c_str());

                                        if (INVALID_HSM_STATE_ID == itHistoryData->second.defaultTarget) {
                                            // transition to parent's entry point if there is no default history target
                                            historyTargets.push_back(historyParent);
                                        } else {
                                            historyTargets.push_back(itHistoryData->second.defaultTarget);
                                            historyTargets.push_back(historyParent);
                                        }
                                    } else {
                                        HSM_TRACE_ERROR("parent for history state=<%s> wasnt found",
                                                        mParent.getStateName(it->destinationState).c_str());
                                    }

                                    PendingEventInfo defHistoryTransitionEvent = event;

                                    defHistoryTransitionEvent.transitionType = TransitionBehavior::FORCED;

                                    for (const StateID_t historyTargetState : historyTargets) {
                                        HsmTransitionCallback_t cbTransition;

                                        defHistoryTransitionEvent.forcedTransitionsInfo =
                                            std::make_shared<std::list<TransitionInfo>>();

                                        if ((INVALID_HSM_STATE_ID != itHistoryData->second.defaultTarget) &&
                                            (historyTargetState == historyParent)) {
                                            defHistoryTransitionEvent.ignoreEntryPoints = true;
                                        } else {
                                            cbTransition = itHistoryData->second.defaultTargetTransitionCallback;
                                        }

                                        defHistoryTransitionEvent.forcedTransitionsInfo->emplace_back(
                                            it->destinationState,
                                            historyTargetState,
                                            TransitionType::EXTERNAL_TRANSITION,
                                            cbTransition,
                                            nullptr);

                                        mPendingEvents.push_front(defHistoryTransitionEvent);
                                    }
                                }

                                res = HsmEventStatus_t::PENDING;
                            }
                            // check if new state has substates and initiate entry transition
                            else if ((false == event.ignoreEntryPoints) &&
                                     (true == getEntryPoints(it->destinationState, event.type, event.args, entryPoints))) {
                                HSM_TRACE_DEBUG("state <%s> has substates with %d entry points (first: <%s>)",
                                                mParent.getStateName(it->destinationState).c_str(),
                                                SC2INT(entryPoints.size()),
                                                mParent.getStateName(entryPoints.front()).c_str());
                                PendingEventInfo entryPointTransitionEvent = event;

                                entryPointTransitionEvent.transitionType = TransitionBehavior::ENTRYPOINT;

                                {
                                    HSM_SYNC_EVENTS_QUEUE();
                                    mPendingEvents.push_front(entryPointTransitionEvent);
                                }
                                res = HsmEventStatus_t::PENDING;
                            } else {
                                if (true == event.ignoreEntryPoints) {
                                    HSM_TRACE_DEBUG(
                                        "entry points were forcefully ignored (probably due to history transition)");
                                    res = HsmEventStatus_t::PENDING;
                                } else {
                                    res = HsmEventStatus_t::DONE_OK;
                                }
                            }
                        }
                    } else {
                        for (auto itState = exitedStates.begin(); itState != exitedStates.end(); ++itState) {
                            // to prevent infinite loops we don't allow state to cancel transition
                            (void)onStateEntering(*itState, VariantVector_t());
                            (void)addActiveState(*itState);
                            onStateChanged(*itState, VariantVector_t());
                        }
                    }
                }
            }
        } else {
            res = HsmEventStatus_t::CANCELED;
        }
    }

    if (HsmEventStatus_t::DONE_FAILED == res) {
        HSM_TRACE_DEBUG("event <%s> in state <%s> was ignored.",
                        mParent.getEventName(event.type).c_str(),
                        mParent.getStateName(fromState).c_str());
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

// ============================================================================
// PRIVATE: PendingEventInfo
// ============================================================================
HierarchicalStateMachine::Impl::PendingEventInfo::~PendingEventInfo() {
    if (true == cvLock.unique()) {
        HSM_TRACE_CALL_DEBUG_ARGS("event=<%d> was deleted. releasing lock", SC2INT(type));
        unlock(HsmEventStatus_t::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

void HierarchicalStateMachine::Impl::PendingEventInfo::initLock() {
    if (!cvLock) {
        cvLock = std::make_shared<Mutex>();
        syncProcessed = std::make_shared<ConditionVariable>();
        transitionStatus = std::make_shared<HsmEventStatus_t>();
        *transitionStatus = HsmEventStatus_t::PENDING;
    }
}

void HierarchicalStateMachine::Impl::PendingEventInfo::releaseLock() {
    if (true == isSync()) {
        HSM_TRACE_CALL_DEBUG_ARGS("releaseLock");
        unlock(HsmEventStatus_t::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

bool HierarchicalStateMachine::Impl::PendingEventInfo::isSync() {
    return (nullptr != cvLock);
}

void HierarchicalStateMachine::Impl::PendingEventInfo::wait(const int timeoutMs) {
    if (true == isSync()) {
        // NOTE: lock is needed only because we have to use cond variable
        UniqueLock lck(*cvLock);

        HSM_TRACE_CALL_DEBUG_ARGS("trying to wait... (current status=%d, %p)",
                                  SC2INT(*transitionStatus),
                                  transitionStatus.get());
        if (timeoutMs > 0) {
            // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
            // cppcheck-suppress [misra-c2012-15.5, misra-c2012-17.7]
            syncProcessed->wait_for(lck, timeoutMs, [=]() { return (HsmEventStatus_t::PENDING != *transitionStatus); });
        } else {
            // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
            // cppcheck-suppress [misra-c2012-15.5, misra-c2012-17.7]
            syncProcessed->wait(lck, [=]() { return (HsmEventStatus_t::PENDING != *transitionStatus); });
        }

        HSM_TRACE_DEBUG("unlocked! transitionStatus=%d", SC2INT(*transitionStatus));
    }
}

void HierarchicalStateMachine::Impl::PendingEventInfo::unlock(const HsmEventStatus_t status) {
    HSM_TRACE_CALL_DEBUG_ARGS("try to unlock with status=%d", SC2INT(status));

    if (true == isSync()) {
        HSM_TRACE_DEBUG("SYNC object (%p)", transitionStatus.get());
        *transitionStatus = status;

        if (status != HsmEventStatus_t::PENDING) {
            syncProcessed->notify();
        }
    } else {
        HSM_TRACE_DEBUG("ASYNC object");
    }
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
    HSM_TRACE_CALL_DEBUG_ARGS("oldState=<%s>, newState=<%s>",
                              mParent.getStateName(oldState).c_str(),
                              mParent.getStateName(newState).c_str());

    if (false == isSubstateOf(oldState, newState)) {
        mActiveStates.remove(oldState);
    }

    return addActiveState(newState);
}

bool HierarchicalStateMachine::Impl::addActiveState(const StateID_t newState) {
    HSM_TRACE_CALL_DEBUG_ARGS("newState=<%s>", mParent.getStateName(newState).c_str());
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

    for (auto itSubstate = mSubstates.begin(); itSubstate != mSubstates.end(); ++itSubstate) {
        if (itSubstate->second == state) {
            result = true;
            break;
        }
    }

    return result;
}

bool HierarchicalStateMachine::Impl::hasParentState(const StateID_t state, StateID_t& outParent) const {
    bool hasParent = false;

    for (auto it = mSubstates.begin(); it != mSubstates.end(); ++it) {
        if (state == it->second) {
            hasParent = true;
            outParent = it->first;  // cppcheck-suppress misra-c2012-17.8 ; outParent is used to return result
            break;
        }
    }

    return hasParent;
}

#endif  // HSM_ENABLE_SAFE_STRUCTURE

bool HierarchicalStateMachine::Impl::enableHsmDebugging() {
#ifdef HSMBUILD_DEBUGGING
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
        char bufTime[80] = {0};
        char bufTimeMs[6] = {0};
        auto currentTimePoint = std::chrono::system_clock::now();
        const std::time_t tt = std::chrono::system_clock::to_time_t(currentTimePoint);
        std::tm timeinfo;
        const std::tm* tmResult = nullptr;  // this is just to check that localtime was executed correctly

  #ifdef WIN32
        if (0 == ::localtime_s(&timeinfo, &tt)) {
            tmResult = &timeinfo;
        }
  #else
        tmResult = localtime(&tt);
        if (nullptr != tmResult) {
            timeinfo = *tmResult;
        }
  #endif  // WIN32

        if (nullptr != tmResult) {
            (void)std::strftime(bufTime, sizeof(bufTime), "%Y-%m-%d %H:%M:%S", &timeinfo);
            (void)snprintf(
                bufTimeMs,
                sizeof(bufTimeMs),
                ".%03d",
                static_cast<int>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint.time_since_epoch()).count() % 1000));
        } else {
            (void)std::strcpy(bufTime, "0000-00-00 00:00:00");
            (void)std::strcpy(bufTimeMs, ".000");
        }

        *mHsmLog << "\n-\n"
                    "  timestamp: \""
                 << bufTime << bufTimeMs
                 << "\"\n"
                    "  active_states:";

        for (auto itState = mActiveStates.begin(); itState != mActiveStates.end(); ++itState) {
            *mHsmLog << "\n    - \"" << mParent.getStateName(*itState) << "\"";
        }

        *mHsmLog << "\n  action: " << actionsMap.at(action)
                 << "\n"
                    "  from_state: \""
                 << mParent.getStateName(fromState)
                 << "\"\n"
                    "  target_state: \""
                 << mParent.getStateName(targetState)
                 << "\"\n"
                    "  event: \""
                 << mParent.getEventName(event)
                 << "\"\n"
                    "  status: "
                 << (hasFailed ? "failed" : "")
                 << "\n"
                    "  args:";

        for (auto itArg = args.begin(); itArg != args.end(); ++itArg) {
            *mHsmLog << "\n    - " << itArg->toString();
        }

        mHsmLog->flush();
    }
#endif  // HSMBUILD_DEBUGGING
}

#ifndef HSM_DISABLE_DEBUG_TRACES
void HierarchicalStateMachine::Impl::dumpActiveStates() {
    HSM_TRACE_CALL();

    std::string temp;

    for (auto it = mActiveStates.begin(); it != mActiveStates.end(); ++it) {
        temp += mParent.getStateName(*it) + std::string(", ");
    }

    HSM_TRACE_DEBUG("active states: <%s>", temp.c_str());
}

#endif  // HSM_DISABLE_DEBUG_TRACES

}  // namespace hsmcpp