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
#include "hsmcpp/os/ConditionVariable.hpp"
#include "hsmcpp/os/AtomicFlag.hpp"
#include "hsmcpp/variant.hpp"
#include "HsmImplTypes.hpp"

namespace hsmcpp {

class IHsmEventDispatcher;

/**
 * @brief Private implementation of HierarchicalStateMachine (pimpl).
 * @details Holds all state-machine logic; the public HierarchicalStateMachine
 * methods forward to the same-named methods here.
 * @implements hsmcpp::HierarchicalStateMachine
 */
class HierarchicalStateMachine::Impl : public std::enable_shared_from_this<HierarchicalStateMachine::Impl> {
public:
    explicit Impl(HierarchicalStateMachine* parent, const StateID_t initialState);
    virtual ~Impl();

    /// @requirement HSMCPP.SWR_HSM_127
    void resetParent();

    void setInitialState(const StateID_t initialState);
    bool initialize(const std::weak_ptr<IHsmEventDispatcher>& dispatcher);
    std::weak_ptr<IHsmEventDispatcher> dispatcher() const;

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
    bool registerStateAction(const StateID_t state,
                             const StateActionTrigger actionTrigger,
                             const StateAction action,
                             VariantVector_t&& args);
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
    void transitionWithArgsArray(const EventID_t event, VariantVector_t&& args);
    bool transitionExWithArgsArray(const EventID_t event,
                                   const bool clearQueue,
                                   const bool sync,
                                   const int timeoutMs,
                                   VariantVector_t&& args);
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
    /// @requirement HSMCPP.SWR_HSM_006
    void createEventHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr, const std::weak_ptr<Impl>& ptrInstance);
    /// @requirement HSMCPP.SWR_HSM_006
    void createTimerHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr, const std::weak_ptr<Impl>& ptrInstance);
    /// @requirement HSMCPP.SWR_HSM_006
    void createEnqueuedEventHandler(const std::shared_ptr<IHsmEventDispatcher>& dispatcherPtr,
                                    const std::weak_ptr<Impl>& ptrInstance);

    /**
     * checks initial state and, if needed, process any automatic initial transitions
     * @requirement HSMCPP.SWR_HSM_006, HSMCPP.SWR_HSM_013
     */
    void handleStartup();

    /// @requirement_wrapper transitionExWithArgsArray
    void transitionSimple(const EventID_t event);

    bool registerSubstate(const StateID_t parent,
                          const StateID_t substate,
                          const bool isEntryPoint,
                          const EventID_t eventCondition = INVALID_HSM_EVENT_ID,
                          HsmTransitionConditionCallback_t conditionCallback = nullptr,
                          const bool expectedConditionValue = true);
    
    /// @requirement HSMCPP.SWR_HSM_068, HSMCPP.SWR_HSM_069, HSMCPP.SWR_HSM_070
    void dispatchEvents();
    /// @requirement HSMCPP.SWR_HSM_075
    void dispatchTimerEvent(const TimerID_t id);
    
    /// @requirement HSMCPP.SWR_HSM_059
    bool onStateExiting(const StateID_t state);
    /// @requirement HSMCPP.SWR_HSM_058
    bool onStateEntering(const StateID_t state, const VariantVector_t& args);
    /// @requirement HSMCPP.SWR_HSM_060
    void onStateChanged(const StateID_t state, const VariantVector_t& args);
    
    /// @requirement HSMCPP.SWR_HSM_062, HSMCPP.SWR_HSM_063, HSMCPP.SWR_HSM_064, HSMCPP.SWR_HSM_065
    void executeStateAction(const StateID_t state, const StateActionTrigger actionTrigger);

    /// @no_requirement Internal state-tree query helper (no observable behavior).
    bool getParentState(const StateID_t child, StateID_t& outParent);
    /// @no_requirement Internal state-tree query helper (no observable behavior).
    bool isSubstateOf(const StateID_t parent, const StateID_t child);
    /// @no_requirement Internal state-tree query helper (no observable behavior).
    bool isFinalState(const StateID_t state) const;
    /// @no_requirement Internal state-tree query helper (no observable behavior).
    bool hasActiveChildren(const StateID_t parent, const bool includeFinal);

    /// @no_requirement Internal history-tree query helper (no observable behavior).
    bool getHistoryParent(const StateID_t historyState, StateID_t& outParent);
    /// @requirement HSMCPP.SWR_HSM_033, HSMCPP.SWR_HSM_036
    void updateHistory(const StateID_t topLevelState, const std::list<StateID_t>& exitedStates);
    
    /// @requirement HSMCPP.SWR_HSM_048
    bool checkTransitionPossibility(const StateID_t fromState, const EventID_t event, const VariantVector_t& args);
    
    /// @requirement HSMCPP.SWR_HSM_048
    bool findTransitionTarget(const StateID_t fromState,
                              const EventID_t event,
                              const VariantVector_t& transitionArgs,
                              const bool searchParents,
                              std::list<TransitionInfo>& outTransitions);
    /**
     * @details Orchestrates a single state's transition: selects the target
     * (determineTargetState), runs self-transitions, exit, and external
     * transition. Pure control-flow orchestration delegating to tagged helpers.
     *
     * @requirement HSMCPP.SWR_HSM_045, HSMCPP.SWR_HSM_057
     */
    HsmEventStatus doTransition(const PendingEventInfo& event);

    /// @requirement HSMCPP.SWR_HSM_054, HSMCPP.SWR_HSM_056
    HsmEventStatus processExternalTransition(const PendingEventInfo& event,
                                             const StateID_t fromState,
                                             const TransitionInfo& curTransition,
                                             const std::list<StateID_t>& exitedStates);
    /**
     * @details Selects the matching transition(s) for the current event and
     * transition behavior (regular/entrypoint/forced).
     *
     * @no_requirement internal method. realizes transition target selection.
     */
    bool determineTargetState(const PendingEventInfo& event,
                              const StateID_t fromState,
                              std::list<TransitionInfo>& outMatchingTransitions);
    /// @requirement HSMCPP.SWR_HSM_043, HSMCPP.SWR_HSM_044
    bool executeSelfTransitions(const PendingEventInfo& event, const std::list<TransitionInfo>& matchingTransitions);
    /// @requirement HSMCPP.SWR_HSM_055
    bool executeExitTransition(const PendingEventInfo& event,
                               const std::list<TransitionInfo>& matchingTransitions,
                               std::list<StateID_t>& outExitedStates);

    /// @requirement HSMCPP.SWR_HSM_032, HSMCPP.SWR_HSM_035, HSMCPP.SWR_HSM_038
    bool processHistoryTransition(const PendingEventInfo& event, const StateID_t destinationState);
    /// @requirement HSMCPP.SWR_HSM_032, HSMCPP.SWR_HSM_035
    void transitionToPreviousActiveStates(std::list<StateID_t>& previousActiveStates, const PendingEventInfo& event, const StateID_t destinationState);
    /// @requirement HSMCPP.SWR_HSM_038
    void transitionToDefaultHistoryState(const StateID_t defaultTarget, const HsmTransitionCallback_t& defaultTargetTransitionCallback, const PendingEventInfo& event, const StateID_t destinationState);


    /**
     * @details When a state's last non-final child reaches a final state, raises
     * the final state's completion event to the parent. For an exit point, the
     * parent is exited with the registered event if one was registered, otherwise
     * with the event that activated the exit point.
     *
     * TODO: need to rename this as it handles both exit points and final states.
     *
     * @requirement HSMCPP.SWR_HSM_115, HSMCPP.SWR_HSM_128, HSMCPP.SWR_HSM_129
     */
    bool processFinalStateTransition(const PendingEventInfo& event, const StateID_t destinationState);
    /**
     * @details Top-level per-state event handler: validates, selects target,
     * executes self/exit/external transition steps. Control-flow orchestration.
     *
     * @requirement HSMCPP.SWR_HSM_054
     */
    HsmEventStatus handleSingleTransition(const StateID_t fromState, const PendingEventInfo& event);
    /// @requirement HSMCPP.SWR_HSM_047
    void clearPendingEvents();

    /// @no_requirement Internal state-tree query helper (no observable behavior).
    bool hasSubstates(const StateID_t parent) const;
    /// @no_requirement Internal state-tree query helper (no observable behavior).
    bool hasEntryPoint(const StateID_t state) const;
    // TODO: return enum instead of bool (no entrypoint registered, no matching entry, ok)
    /// @requirement HSMCPP.SWR_HSM_020, HSMCPP.SWR_HSM_021, HSMCPP.SWR_HSM_022, HSMCPP.SWR_HSM_023, HSMCPP.SWR_HSM_024
    bool getEntryPoints(const StateID_t state,
                        const EventID_t onEvent,
                        const VariantVector_t& transitionArgs,
                        std::list<StateID_t>& outEntryPoints) const;

    // returns TRUE if newState was added to a list of active states
    /// @no_requirement Internal active-state bookkeeping (no observable behavior).
    bool replaceActiveState(const StateID_t oldState, const StateID_t newState);
    // returns TRUE if newState was added to a list of active states
    /// @no_requirement Internal active-state bookkeeping (no observable behavior).
    bool addActiveState(const StateID_t newState);

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    bool isTopState(const StateID_t state) const;
    bool isSubstate(const StateID_t state) const;
    bool hasParentState(const StateID_t state, StateID_t& outParent) const;
#endif  // HSM_ENABLE_SAFE_STRUCTURE

    /// @requirement HSMCPP.SWR_HSM_099
    void logHsmAction(const HsmLogAction action,
                      const StateID_t fromState = INVALID_HSM_STATE_ID,
                      const StateID_t targetState = INVALID_HSM_STATE_ID,
                      const EventID_t event = INVALID_HSM_EVENT_ID,
                      const bool hasFailed = false,
                      const VariantVector_t& args = VariantVector_t());

#ifndef HSM_DISABLE_DEBUG_TRACES
    /// @no_requirement Debug-trace-only diagnostic dump (compiled out in release).
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
    AtomicFlag mIsDispatching;
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
