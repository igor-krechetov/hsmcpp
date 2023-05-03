// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_SRC_HSMIMPL_HPP
#define HSMCPP_SRC_HSMIMPL_HPP

#include <list>
#include <map>
#include <memory>
#include <vector>
#ifdef HSMBUILD_DEBUGGING
  #include <fstream>
#endif

#include "hsmcpp/hsm.hpp"
#include "hsmcpp/os/Mutex.hpp"
#include "hsmcpp/variant.hpp"
#include "HsmImplTypes.hpp"

namespace hsmcpp {

class IHsmEventDispatcher;

class HierarchicalStateMachine::Impl : public std::enable_shared_from_this<HierarchicalStateMachine::Impl> {
public:
    explicit Impl(HierarchicalStateMachine* parent, const StateID_t initialState);
    virtual ~Impl();

    void resetParent();

    void setInitialState(const StateID_t initialState);
    bool initialize(const std::weak_ptr<IHsmEventDispatcher>& dispatcher);
    bool isInitialized() const;
    void release();
    void registerFailedTransitionCallback(HsmTransitionFailedCallback_t onFailedTransition);
    void registerState(const StateID_t state,
                       HsmStateChangedCallback_t onStateChanged = nullptr,
                       HsmStateEnterCallback_t onEntering = nullptr,
                       HsmStateExitCallback_t onExiting = nullptr);
    void registerFinalState(const StateID_t state,
                            const EventID_t event = INVALID_HSM_EVENT_ID,
                            HsmStateChangedCallback_t onStateChanged = nullptr,
                            HsmStateEnterCallback_t onEntering = nullptr,
                            HsmStateExitCallback_t onExiting = nullptr);
    void registerHistory(const StateID_t parent,
                         const StateID_t historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const StateID_t defaultTarget = INVALID_HSM_STATE_ID,
                         HsmTransitionCallback_t transitionCallback = nullptr);
    bool registerSubstate(const StateID_t parent, const StateID_t substate);
    bool registerSubstateEntryPoint(const StateID_t parent,
                                    const StateID_t substate,
                                    const EventID_t onEvent = INVALID_HSM_EVENT_ID,
                                    HsmTransitionConditionCallback_t conditionCallback = nullptr,
                                    const bool expectedConditionValue = true);
    void registerTimer(const TimerID_t timerID, const EventID_t event);
    bool registerStateAction(const StateID_t state,
                             const StateActionTrigger actionTrigger,
                             const StateAction action,
                             const VariantVector_t& args);
    void registerTransition(const StateID_t from,
                            const StateID_t to,
                            const EventID_t onEvent,
                            HsmTransitionCallback_t transitionCallback = nullptr,
                            HsmTransitionConditionCallback_t conditionCallback = nullptr,
                            const bool expectedConditionValue = true);
    void registerSelfTransition(const StateID_t state,
                                const EventID_t onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmTransitionCallback_t transitionCallback = nullptr,
                                HsmTransitionConditionCallback_t conditionCallback = nullptr,
                                const bool expectedConditionValue = true);
    StateID_t getLastActiveState() const;
    const std::list<StateID_t>& getActiveStates() const;
    bool isStateActive(const StateID_t state) const;

    void transitionWithArgsArray(const EventID_t event, const VariantVector_t& args);
    bool transitionExWithArgsArray(const EventID_t event,
                                   const bool clearQueue,
                                   const bool sync,
                                   const int timeoutMs,
                                   const VariantVector_t& args);
    bool transitionInterruptSafe(const EventID_t event);
    bool isTransitionPossible(const EventID_t event, const VariantVector_t& args);
    void startTimer(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot);
    void restartTimer(const TimerID_t timerID);
    void stopTimer(const TimerID_t timerID);
    bool isTimerRunning(const TimerID_t timerID);
    bool enableHsmDebugging();
    bool enableHsmDebugging(const std::string& dumpPath);
    void disableHsmDebugging();

private:
    void createEventHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr, const std::weak_ptr<Impl>& ptrInstance);
    void createTimerHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr, const std::weak_ptr<Impl>& ptrInstance);
    void createEnqueuedEventHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr,
                                    const std::weak_ptr<Impl>& ptrInstance);

    // checks initial state and, if needed, process any automatic initial transitions
    void handleStartup();

    void transitionSimple(const EventID_t event);

    bool registerSubstate(const StateID_t parent,
                          const StateID_t substate,
                          const bool isEntryPoint,
                          const EventID_t eventCondition = INVALID_HSM_EVENT_ID,
                          HsmTransitionConditionCallback_t conditionCallback = nullptr,
                          const bool expectedConditionValue = true);

    void dispatchEvents();
    void dispatchTimerEvent(const TimerID_t id);

    bool onStateExiting(const StateID_t state);
    bool onStateEntering(const StateID_t state, const VariantVector_t& args);
    void onStateChanged(const StateID_t state, const VariantVector_t& args);

    void executeStateAction(const StateID_t state, const StateActionTrigger actionTrigger);

    bool getParentState(const StateID_t child, StateID_t& outParent);
    bool isSubstateOf(const StateID_t parent, const StateID_t child);
    bool isFinalState(const StateID_t state) const;
    bool hasActiveChildren(const StateID_t parent, const bool includeFinal);

    bool getHistoryParent(const StateID_t historyState, StateID_t& outParent);
    void updateHistory(const StateID_t topLevelState, const std::list<StateID_t>& exitedStates);

    bool checkTransitionPossibility(const StateID_t fromState, const EventID_t event, const VariantVector_t& args);

    bool findTransitionTarget(const StateID_t fromState,
                              const EventID_t event,
                              const VariantVector_t& transitionArgs,
                              const bool searchParents,
                              std::list<TransitionInfo>& outTransitions);
    HsmEventStatus doTransition(const PendingEventInfo& event);

    HsmEventStatus processExternalTransition(const PendingEventInfo& event,
                                             const StateID_t fromState,
                                             const TransitionInfo& curTransition,
                                             const std::list<StateID_t>& exitedStates);
    bool determineTargetState(const PendingEventInfo& event,
                              const StateID_t fromState,
                              std::list<TransitionInfo>& outMatchingTransitions);
    bool executeSelfTransitions(const PendingEventInfo& event, const std::list<TransitionInfo>& matchingTransitions);
    bool executeExitTransition(const PendingEventInfo& event,
                               const std::list<TransitionInfo>& matchingTransitions,
                               std::list<StateID_t>& outExitedStates);

    bool processHistoryTransition(const PendingEventInfo& event, const StateID_t destinationState);
    void transitionToPreviousActiveStates(std::list<StateID_t>& previousActiveStates, const PendingEventInfo& event, const StateID_t destinationState);
    void transitionToDefaultHistoryState(const StateID_t defaultTarget, const HsmTransitionCallback_t& defaultTargetTransitionCallback, const PendingEventInfo& event, const StateID_t destinationState);


    bool processFinalStateTransition(const PendingEventInfo& event, const StateID_t destinationState);
    HsmEventStatus handleSingleTransition(const StateID_t fromState, const PendingEventInfo& event);
    void clearPendingEvents();

    bool hasSubstates(const StateID_t parent) const;
    bool hasEntryPoint(const StateID_t state) const;
    // TODO: return enum instead of bool (no entrypoint registered, no matching entry, ok)
    bool getEntryPoints(const StateID_t state,
                        const EventID_t onEvent,
                        const VariantVector_t& transitionArgs,
                        std::list<StateID_t>& outEntryPoints) const;

    // returns TRUE if newState was added to a list of active states
    bool replaceActiveState(const StateID_t oldState, const StateID_t newState);
    // returns TRUE if newState was added to a list of active states
    bool addActiveState(const StateID_t newState);

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    bool isTopState(const StateID_t state) const;
    bool isSubstate(const StateID_t state) const;
    bool hasParentState(const StateID_t state, StateID_t& outParent) const;
#endif  // HSM_ENABLE_SAFE_STRUCTURE

    void logHsmAction(const HsmLogAction action,
                      const StateID_t fromState = INVALID_HSM_STATE_ID,
                      const StateID_t targetState = INVALID_HSM_STATE_ID,
                      const EventID_t event = INVALID_HSM_EVENT_ID,
                      const bool hasFailed = false,
                      const VariantVector_t& args = VariantVector_t());

#ifndef HSM_DISABLE_DEBUG_TRACES
    void dumpActiveStates();
#endif

    std::string getStateName(const StateID_t state);
    std::string getEventName(const EventID_t event);

private:
    HierarchicalStateMachine* mParent = nullptr;
    std::weak_ptr<IHsmEventDispatcher> mDispatcher;  // protected by mParentSync
    HandlerID_t mEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    HandlerID_t mEnqueuedEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    HandlerID_t mTimerHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    bool mStopDispatching = false;

    HsmTransitionFailedCallback_t mFailedTransitionCallback;

    StateID_t mInitialState;
    std::list<StateID_t> mActiveStates;
    std::multimap<std::pair<StateID_t, EventID_t>, TransitionInfo> mTransitionsByEvent;  // FROM_STATE, EVENT => TO
    std::map<StateID_t, StateCallbacks> mRegisteredStates;
    std::map<StateID_t, EventID_t> mFinalStates;
    std::multimap<StateID_t, StateID_t> mSubstates;
    std::multimap<StateID_t, StateEntryPoint> mSubstateEntryPoints;
    std::list<PendingEventInfo> mPendingEvents;  // protected by mEventsSync
    std::map<TimerID_t, EventID_t> mTimers;

    // parent state, history state
    std::multimap<StateID_t, StateID_t> mHistoryStates;
    // history state id, data
    std::map<StateID_t, HistoryInfo> mHistoryData;

    std::multimap<std::pair<StateID_t, StateActionTrigger>, StateActionInfo> mRegisteredActions;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    std::list<StateID_t> mTopLevelStates;  // list of states which are not substates and dont have substates of their own
#endif

#ifndef HSM_DISABLE_THREADSAFETY
    Mutex mEventsSync;
  #if !defined(HSM_DISABLE_DEBUG_TRACES)
    Mutex mParentSync;
  #endif
#endif  // HSM_DISABLE_THREADSAFETY

#ifdef HSMBUILD_DEBUGGING
    std::filebuf mHsmLogFile;
    std::shared_ptr<std::ostream> mHsmLog;
#endif  // HSMBUILD_DEBUGGING
};

}  // namespace hsmcpp

#endif  // HSMCPP_SRC_HSMIMPL_HPP
