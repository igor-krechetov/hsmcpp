// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSM_HPP
#define HSMCPP_HSM_HPP

#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "IHsmEventDispatcher.hpp"
#include "os/ConditionVariable.hpp"
#include "os/LockGuard.hpp"
#include "os/Mutex.hpp"
#include "variant.hpp"
// TODO: use logging.hpp from external repo
#include "logging.hpp"

#if !defined(HSM_DISABLE_THREADSAFETY) && defined(FREERTOS_AVAILABLE)
  #include "os/InterruptsFreeSection.hpp"
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

namespace hsmcpp {
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

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HierarchicalStateMachine"

#define HSM_WAIT_INDEFINITELY (0)
#define INVALID_ID (-1000)
#define INVALID_HSM_EVENT_ID static_cast<HsmEventEnum>(INVALID_ID)
#define INVALID_HSM_STATE_ID static_cast<HsmStateEnum>(INVALID_ID)

#define ENV_DUMPPATH "HSMCPP_DUMP_PATH"
#define DEFAULT_DUMP_PATH "./dump.hsmlog"

template <typename HsmStateEnum, typename HsmEventEnum>
class HierarchicalStateMachine {
public:
    using HsmTransitionCallback_t = std::function<void(const VariantVector_t&)>;
    using HsmTransitionConditionCallback_t = std::function<bool(const VariantVector_t&)>;
    using HsmStateChangedCallback_t = std::function<void(const VariantVector_t&)>;
    using HsmStateEnterCallback_t = std::function<bool(const VariantVector_t&)>;
    using HsmStateExitCallback_t = std::function<bool(void)>;
    using HsmTransitionFailedCallback_t = std::function<void(const HsmEventEnum, const VariantVector_t&)>;

// NOTE: enclosing input expressions in parentheses is not needed (and will not compile)
// cppcheck-suppress misra-c2012-20.7
#define HsmTransitionCallbackPtr_t(_class, _func) void (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmTransitionConditionCallbackPtr_t(_class, _func) bool (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmStateChangedCallbackPtr_t(_class, _func) void (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmStateEnterCallbackPtr_t(_class, _func) bool (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmStateExitCallbackPtr_t(_class, _func) bool (_class::*_func)()
// cppcheck-suppress misra-c2012-20.7
#define HsmTransitionFailedCallbackPtr_t(_class, _func) void (_class::*_func)(const HsmEventEnum, const VariantVector_t&)

    enum class HistoryType { SHALLOW, DEEP };

    enum class TransitionType { INTERNAL_TRANSITION, EXTERNAL_TRANSITION };

    enum class StateActionTrigger { ON_STATE_ENTRY, ON_STATE_EXIT };

    enum class StateAction {
        START_TIMER,    // ARGS: TimerID_t timerID, int32_t intervalMs, bool singleshot
        STOP_TIMER,     // ARGS: TimerID_t timerID
        RESTART_TIMER,  // ARGS: TimerID_t timerID
        TRANSITION,     // ARGS: EventID_t eventID
    };

private:
    enum class HsmLogAction {
        IDLE,
        TRANSITION,
        TRANSITION_ENTRYPOINT,
        CALLBACK_EXIT,
        CALLBACK_ENTER,
        CALLBACK_STATE,
        ON_ENTER_ACTIONS,
        ON_EXIT_ACTIONS,
    };

    enum class HsmEventStatus { PENDING, DONE_OK, DONE_FAILED, CANCELED };

    enum class TransitionBehavior { REGULAR, ENTRYPOINT, FORCED };

// NOTE: just an alias to make code more readable
#if defined(WIN32) && (__cplusplus == 201103L)
    // only for Win32 with C++11
  #define HsmEventStatus_t typename HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HsmEventStatus
#else
  #define HsmEventStatus_t HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HsmEventStatus
#endif

    struct StateCallbacks {
        HsmStateChangedCallback_t onStateChanged = nullptr;
        HsmStateEnterCallback_t onEntering = nullptr;
        HsmStateExitCallback_t onExiting = nullptr;
    };

    struct StateEntryPoint {
        HsmStateEnum state = INVALID_HSM_STATE_ID;
        HsmEventEnum onEvent = INVALID_HSM_EVENT_ID;
        HsmTransitionConditionCallback_t checkCondition = nullptr;
        bool expectedConditionValue = true;
    };

    struct TransitionInfo {
        HsmStateEnum fromState = INVALID_HSM_STATE_ID;
        HsmStateEnum destinationState = INVALID_HSM_STATE_ID;
        TransitionType transitionType = TransitionType::EXTERNAL_TRANSITION;
        HsmTransitionCallback_t onTransition = nullptr;
        HsmTransitionConditionCallback_t checkCondition = nullptr;
        bool expectedConditionValue = true;

        TransitionInfo() = default;

        TransitionInfo(const HsmStateEnum from,
                       const HsmStateEnum to,
                       const TransitionType type,
                       const HsmTransitionCallback_t& cbTransition,
                       const HsmTransitionConditionCallback_t& cbCondition)
            : fromState(from)
            , destinationState(to)
            , transitionType(type)
            , onTransition(cbTransition)
            , checkCondition(cbCondition)
            , expectedConditionValue(true) {}

        TransitionInfo(const HsmStateEnum from,
                       const HsmStateEnum to,
                       const TransitionType type,
                       const HsmTransitionCallback_t& cbTransition,
                       const HsmTransitionConditionCallback_t& cbCondition,
                       const bool conditionValue)
            : fromState(from)
            , destinationState(to)
            , transitionType(type)
            , onTransition(cbTransition)
            , checkCondition(cbCondition)
            , expectedConditionValue(conditionValue) {}
    };

    struct PendingEventInfo {
        TransitionBehavior transitionType = TransitionBehavior::REGULAR;
        HsmEventEnum type = INVALID_HSM_EVENT_ID;
        VariantVector_t args;
        std::shared_ptr<Mutex> cvLock;
        std::shared_ptr<ConditionVariable> syncProcessed;
        std::shared_ptr<HsmEventStatus> transitionStatus;
        std::shared_ptr<std::list<TransitionInfo>> forcedTransitionsInfo;
        bool ignoreEntryPoints = false;

        ~PendingEventInfo();
        void initLock();
        void releaseLock();
        bool isSync();
        void wait(const int timeoutMs = HSM_WAIT_INDEFINITELY);
        void unlock(const HsmEventStatus status);
    };

    struct HistoryInfo {
        HistoryType type = HistoryType::SHALLOW;
        HsmStateEnum defaultTarget = INVALID_HSM_STATE_ID;
        HsmTransitionCallback_t defaultTargetTransitionCallback = nullptr;
        std::list<HsmStateEnum> previousActiveStates;

        HistoryInfo(const HistoryType newType,
                    const HsmStateEnum newDefaultTarget,
                    HsmTransitionCallback_t newTransitionCallback)
            : type(newType)
            , defaultTarget(newDefaultTarget)
            , defaultTargetTransitionCallback(newTransitionCallback) {}
    };

    struct StateActionInfo {
        StateAction action;
        VariantVector_t actionArgs;
    };

public:
    explicit HierarchicalStateMachine(const HsmStateEnum initialState);
    // Uses unregisterEventHandler from Dispatcher. Usually HSM has to be destroyed from the same thread it was created.
    virtual ~HierarchicalStateMachine();

    // changes initial state of HSM
    //
    // NOTE: can be called only before initialize()
    void setInitialState(const HsmStateEnum initialState);

    // Uses registerEventHandler from Dispatcher. Usually must be called from the same thread where dispatcher was created.
    //
    // NOTE: after calling this function HSM becomes operable. So HSM structure must be registered BEFORE calling it.
    //       changing structure after this call can result in undefined behaviour and is not advised.
    virtual bool initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher);

    inline bool isInitialized() const;

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
    void registerState(const HsmStateEnum state,
                       HsmHandlerClass* handler = nullptr,
                       HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged) = nullptr,
                       HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering) = nullptr,
                       HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting) = nullptr);

    void registerState(const HsmStateEnum state,
                       HsmStateChangedCallback_t onStateChanged = nullptr,
                       HsmStateEnterCallback_t onEntering = nullptr,
                       HsmStateExitCallback_t onExiting = nullptr);

    template <class HsmHandlerClass>
    void registerFinalState(const HsmStateEnum state,
                            const HsmEventEnum event = INVALID_HSM_EVENT_ID,
                            HsmHandlerClass* handler = nullptr,
                            HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged) = nullptr,
                            HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering) = nullptr,
                            HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting) = nullptr);

    void registerFinalState(const HsmStateEnum state,
                            const HsmEventEnum event = INVALID_HSM_EVENT_ID,
                            HsmStateChangedCallback_t onStateChanged = nullptr,
                            HsmStateEnterCallback_t onEntering = nullptr,
                            HsmStateExitCallback_t onExiting = nullptr);

    // TODO: check structure and return FALSE?
    template <class HsmHandlerClass>
    void registerHistory(const HsmStateEnum parent,
                         const HsmStateEnum historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const HsmStateEnum defaultTarget = INVALID_HSM_STATE_ID,
                         HsmHandlerClass* handler = nullptr,
                         HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr);

    void registerHistory(const HsmStateEnum parent,
                         const HsmStateEnum historyState,
                         const HistoryType type = HistoryType::SHALLOW,
                         const HsmStateEnum defaultTarget = INVALID_HSM_STATE_ID,
                         HsmTransitionCallback_t transitionCallback = nullptr);

    bool registerSubstate(const HsmStateEnum parent, const HsmStateEnum substate);
    template <class HsmHandlerClass>
    bool registerSubstateEntryPoint(const HsmStateEnum parent,
                                    const HsmStateEnum substate,
                                    const HsmEventEnum onEvent = INVALID_HSM_EVENT_ID,
                                    HsmHandlerClass* handler = nullptr,
                                    HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                                    const bool expectedConditionValue = true);
    bool registerSubstateEntryPoint(const HsmStateEnum parent,
                                    const HsmStateEnum substate,
                                    const HsmEventEnum onEvent = INVALID_HSM_EVENT_ID,
                                    const HsmTransitionConditionCallback_t& conditionCallback = nullptr,
                                    const bool expectedConditionValue = true);

    void registerTimer(const TimerID_t timerID, const HsmEventEnum event);

    // TODO: add support for transition actions
    template <typename... Args>
    bool registerStateAction(const HsmStateEnum state,
                             const StateActionTrigger actionTrigger,
                             const StateAction action,
                             Args... args);

    template <class HsmHandlerClass>
    void registerTransition(const HsmStateEnum from,
                            const HsmStateEnum to,
                            const HsmEventEnum onEvent,
                            HsmHandlerClass* handler = nullptr,
                            HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                            HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                            const bool expectedConditionValue = true);

    void registerTransition(const HsmStateEnum from,
                            const HsmStateEnum to,
                            const HsmEventEnum onEvent,
                            HsmTransitionCallback_t transitionCallback = nullptr,
                            HsmTransitionConditionCallback_t conditionCallback = nullptr,
                            const bool expectedConditionValue = true);

    template <class HsmHandlerClass>
    void registerSelfTransition(const HsmStateEnum state,
                                const HsmEventEnum onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmHandlerClass* handler = nullptr,
                                HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                                HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr,
                                const bool expectedConditionValue = true);

    void registerSelfTransition(const HsmStateEnum state,
                                const HsmEventEnum onEvent,
                                const TransitionType type = TransitionType::EXTERNAL_TRANSITION,
                                HsmTransitionCallback_t transitionCallback = nullptr,
                                HsmTransitionConditionCallback_t conditionCallback = nullptr,
                                const bool expectedConditionValue = true);

    // If HSM doesnt contain any parallel states returns current active state.
    // Otherwise returns most recently activated state
    HsmStateEnum getLastActiveState() const;
    const std::list<HsmStateEnum>& getActiveStates() const;
    bool isStateActive(const HsmStateEnum state) const;

    // extended version of transition function with all possible arguments
    // use HSM_WAIT_INDEFINITELY for timeoutMs to ignore timeout
    template <typename... Args>
    bool transitionEx(const HsmEventEnum event, const bool clearQueue, const bool sync, const int timeoutMs, Args... args);

    // basic async transition
    template <typename... Args>
    void transition(const HsmEventEnum event, Args... args);

    // same as regular functions, but uses VariantVector_t for arguments
    void transitionWithArgsArray(const HsmEventEnum event, const VariantVector_t& args);
    bool transitionExWithArgsArray(const HsmEventEnum event,
                                   const bool clearQueue,
                                   const bool sync,
                                   const int timeoutMs,
                                   const VariantVector_t& args);

    // sync transition
    template <typename... Args>
    bool transitionSync(const HsmEventEnum event, const int timeoutMs, Args... args);

    // async transition which clears events queue before adding requested event
    template <typename... Args>
    void transitionWithQueueClear(const HsmEventEnum event, Args... args);

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
    bool transitionInterruptSafe(const HsmEventEnum event);

    template <typename... Args>
    bool isTransitionPossible(const HsmEventEnum event, Args... args);

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

private:
    // checks initial state and, if needed, process any automatic initial transitions
    void handleStartup();

    template <typename... Args>
    void makeVariantList(VariantVector_t& vList, Args&&... args);

    bool registerSubstate(const HsmStateEnum parent,
                          const HsmStateEnum substate,
                          const bool isEntryPoint,
                          const HsmEventEnum eventCondition = INVALID_HSM_EVENT_ID,
                          const HsmTransitionConditionCallback_t& conditionCallback = nullptr,
                          const bool expectedConditionValue = true);

    void dispatchEvents();
    void dispatchTimerEvent(const TimerID_t id);

    bool onStateExiting(const HsmStateEnum state);
    bool onStateEntering(const HsmStateEnum state, const VariantVector_t& args);
    void onStateChanged(const HsmStateEnum state, const VariantVector_t& args);

    void executeStateAction(const HsmStateEnum state, const StateActionTrigger actionTrigger);

    bool getParentState(const HsmStateEnum child, HsmStateEnum& outParent);
    bool isSubstateOf(const HsmStateEnum parent, const HsmStateEnum child);

    bool getHistoryParent(const HsmStateEnum historyState, HsmStateEnum& outParent);
    void updateHistory(const HsmStateEnum topLevelState, const std::list<HsmStateEnum>& exitedStates);

    template <typename... Args>
    bool checkTransitionPossibility(const HsmStateEnum fromState, const HsmEventEnum event, Args... args);

    bool findTransitionTarget(const HsmStateEnum fromState,
                              const HsmEventEnum event,
                              const VariantVector_t& transitionArgs,
                              const bool searchParents,
                              std::list<TransitionInfo>& outTransitions);
    HsmEventStatus doTransition(const PendingEventInfo& event);
    HsmEventStatus handleSingleTransition(const HsmStateEnum fromState, const PendingEventInfo& event);
    void clearPendingEvents();

    bool hasSubstates(const HsmStateEnum parent) const;
    bool hasEntryPoint(const HsmStateEnum state) const;
    // TODO: return enum instead of bool (no entrypoint registered, no matching entry, ok)
    bool getEntryPoints(const HsmStateEnum state,
                        const HsmEventEnum onEvent,
                        const VariantVector_t& transitionArgs,
                        std::list<HsmStateEnum>& outEntryPoints) const;

    // returns TRUE if newState was added to a list of active states
    bool replaceActiveState(const HsmStateEnum oldState, const HsmStateEnum newState);
    // returns TRUE if newState was added to a list of active states
    bool addActiveState(const HsmStateEnum newState);

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    bool isTopState(const HsmStateEnum state) const;
    bool isSubstate(const HsmStateEnum state) const;
    bool hasParentState(const HsmStateEnum state, HsmStateEnum& outParent) const;
#endif  // HSM_ENABLE_SAFE_STRUCTURE

    void logHsmAction(const HsmLogAction action,
                      const HsmStateEnum fromState = INVALID_HSM_STATE_ID,
                      const HsmStateEnum targetState = INVALID_HSM_STATE_ID,
                      const HsmEventEnum event = INVALID_HSM_EVENT_ID,
                      const bool hasFailed = false,
                      const VariantVector_t& args = VariantVector_t());

#ifndef HSM_DISABLE_DEBUG_TRACES
  #define DEBUG_DUMP_ACTIVE_STATES() dumpActiveStates()

    void dumpActiveStates() {
        HSM_TRACE_CALL();

        std::string temp;

        for (auto it = mActiveStates.begin(); it != mActiveStates.end(); ++it) {
            temp += getStateName(*it) + std::string(", ");
        }

        HSM_TRACE_DEBUG("active states: <%s>", temp.c_str());
    }
#else
  #define DEBUG_DUMP_ACTIVE_STATES()
#endif

protected:
    // NOTE: clients must implement this method for debugging to work. names should match with the names in scxml file
    virtual std::string getStateName(const HsmStateEnum state) const;
    virtual std::string getEventName(const HsmEventEnum event) const;

private:
    std::shared_ptr<IHsmEventDispatcher> mDispatcher;
    HandlerID_t mEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    HandlerID_t mEnqueuedEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    HandlerID_t mTimerHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    bool mStopDispatching = false;

    HsmTransitionFailedCallback_t mFailedTransitionCallback;

    HsmStateEnum mInitialState;
    std::list<HsmStateEnum> mActiveStates;
    std::multimap<std::pair<HsmStateEnum, HsmEventEnum>, TransitionInfo> mTransitionsByEvent;  // FROM_STATE, EVENT => TO
    std::map<HsmStateEnum, StateCallbacks> mRegisteredStates;
    std::map<HsmStateEnum, HsmEventEnum> mFinalStates;
    std::multimap<HsmStateEnum, HsmStateEnum> mSubstates;
    std::multimap<HsmStateEnum, StateEntryPoint> mSubstateEntryPoints;
    std::list<PendingEventInfo> mPendingEvents;
    std::map<TimerID_t, HsmEventEnum> mTimers;

    // parent state, history state
    std::multimap<HsmStateEnum, HsmStateEnum> mHistoryStates;
    // history state id, data
    std::map<HsmStateEnum, HistoryInfo> mHistoryData;

    std::multimap<std::pair<HsmStateEnum, StateActionTrigger>, StateActionInfo> mRegisteredActions;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    std::list<HsmStateEnum> mTopLevelStates;  // list of states which are not substates and dont have substates of their own
#endif                                        // HSM_ENABLE_SAFE_STRUCTURE

#ifndef HSM_DISABLE_THREADSAFETY
    Mutex mEventsSync;
#endif  // HSM_DISABLE_THREADSAFETY

#ifdef HSMBUILD_DEBUGGING
    std::filebuf mHsmLogFile;
    std::shared_ptr<std::ostream> mHsmLog;
#endif  // HSMBUILD_DEBUGGING
};

// ============================================================================
// PUBLIC
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HierarchicalStateMachine(const HsmStateEnum initialState)
    : mInitialState(initialState) {
    HSM_TRACE_INIT();
}

template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::~HierarchicalStateMachine() {
    release();
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::setInitialState(const HsmStateEnum initialState) {
    if (!mDispatcher) {
        mInitialState = initialState;
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher) {
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    if (!mDispatcher) {
        // NOTE: false-positive. std::shated_ptr has a bool() operator
        // cppcheck-suppress misra-c2012-14.4
        if (dispatcher) {
            if (true == dispatcher->start()) {
                mDispatcher = dispatcher;
                mEventsHandlerId = mDispatcher->registerEventHandler(
                    std::bind(&HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchEvents, this));
                mTimerHandlerId = mDispatcher->registerTimerHandler(
                    std::bind(&HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchTimerEvent,
                              this,
                              std::placeholders::_1));
                mEnqueuedEventsHandlerId = mDispatcher->registerEnqueuedEventHandler(
                    [&](const EventID_t event) { transition(static_cast<HsmEventEnum>(event)); });

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

template <typename HsmStateEnum, typename HsmEventEnum>
inline bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isInitialized() const {
    return (nullptr != mDispatcher);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::release() {
    mStopDispatching = true;
    HSM_TRACE_CALL_DEBUG();

    disableHsmDebugging();

    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->unregisterEventHandler(mEventsHandlerId);
        mDispatcher->unregisterEnqueuedEventHandler(mEnqueuedEventsHandlerId);
        mDispatcher->unregisterTimerHandler(mTimerHandlerId);
        mDispatcher.reset();
        mEventsHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerFailedTransitionCallback(
    const HsmTransitionFailedCallback_t& onFailedTransition) {
    mFailedTransitionCallback = onFailedTransition;
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerFailedTransitionCallback(
    HsmHandlerClass* handler,
    HsmTransitionFailedCallbackPtr_t(HsmHandlerClass, onFailedTransition)) {
    mFailedTransitionCallback = std::bind(onFailedTransition, handler, std::placeholders::_1, std::placeholders::_2);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerState(
    const HsmStateEnum state,
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

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerState(const HsmStateEnum state,
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

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerFinalState(
    const HsmStateEnum state,
    const HsmEventEnum event,
    HsmHandlerClass* handler,
    HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged),
    HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering),
    HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting)) {
    mFinalStates.emplace(state, event);
    registerState(state, handler, onStateChanged, onEntering, onExiting);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerFinalState(const HsmStateEnum state,
                                                                              const HsmEventEnum event,
                                                                              HsmStateChangedCallback_t onStateChanged,
                                                                              HsmStateEnterCallback_t onEntering,
                                                                              HsmStateExitCallback_t onExiting) {
    mFinalStates.emplace(state, event);
    registerState(state, onStateChanged, onEntering, onExiting);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerHistory(const HsmStateEnum parent,
                                                                           const HsmStateEnum historyState,
                                                                           const HistoryType type,
                                                                           const HsmStateEnum defaultTarget,
                                                                           HsmHandlerClass* handler,
                                                                           HsmTransitionCallbackPtr_t(HsmHandlerClass,
                                                                                                      transitionCallback)) {
    HsmTransitionCallback_t funcTransitionCallback;

    if (nullptr != handler) {
        if (nullptr != transitionCallback) {
            funcTransitionCallback = std::bind(transitionCallback, handler, std::placeholders::_1);
        }
    }

    registerHistory(parent, historyState, type, defaultTarget, funcTransitionCallback);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerHistory(const HsmStateEnum parent,
                                                                           const HsmStateEnum historyState,
                                                                           const HistoryType type,
                                                                           const HsmStateEnum defaultTarget,
                                                                           HsmTransitionCallback_t transitionCallback) {
    (void)mHistoryStates.emplace(parent, historyState);
    mHistoryData.emplace(historyState, HistoryInfo(type, defaultTarget, transitionCallback));
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstate(const HsmStateEnum parent,
                                                                            const HsmStateEnum substate) {
    return registerSubstate(parent, substate, false);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstateEntryPoint(
    const HsmStateEnum parent,
    const HsmStateEnum substate,
    const HsmEventEnum onEvent,
    HsmHandlerClass* handler,
    HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback),
    const bool expectedConditionValue) {
    HsmTransitionConditionCallback_t condition;

    if ((nullptr != handler) && (nullptr != conditionCallback)) {
        condition = std::bind(conditionCallback, handler, std::placeholders::_1);
    }

    return registerSubstateEntryPoint(parent, substate, onEvent, condition, expectedConditionValue);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstateEntryPoint(
    const HsmStateEnum parent,
    const HsmStateEnum substate,
    const HsmEventEnum onEvent,
    const HsmTransitionConditionCallback_t& conditionCallback,
    const bool expectedConditionValue) {
    return registerSubstate(parent, substate, true, onEvent, conditionCallback, expectedConditionValue);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerTimer(const TimerID_t timerID, const HsmEventEnum event) {
    mTimers.emplace(timerID, event);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstate(
    const HsmStateEnum parent,
    const HsmStateEnum substate,
    const bool isEntryPoint,
    const HsmEventEnum onEvent,
    const HsmTransitionConditionCallback_t& conditionCallback,
    const bool expectedConditionValue) {
    bool registrationAllowed = false;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    // do a simple sanity check
    if (parent != substate) {
        HsmStateEnum curState = parent;
        HsmStateEnum prevState;

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

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerStateAction(const HsmStateEnum state,
                                                                               const StateActionTrigger actionTrigger,
                                                                               const StateAction action,
                                                                               Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>, actionTrigger=%d, action=%d",
                              getStateName(state).c_str(),
                              SC2INT(actionTrigger),
                              SC2INT(action));
    bool result = false;
    bool argsValid = false;
    StateActionInfo newAction;

    makeVariantList(newAction.actionArgs, args...);

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

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerTransition(
    const HsmStateEnum from,
    const HsmStateEnum to,
    const HsmEventEnum onEvent,
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

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerTransition(
    const HsmStateEnum from,
    const HsmStateEnum to,
    const HsmEventEnum onEvent,
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

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSelfTransition(
    const HsmStateEnum state,
    const HsmEventEnum onEvent,
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

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSelfTransition(
    const HsmStateEnum state,
    const HsmEventEnum onEvent,
    const TransitionType type,
    HsmTransitionCallback_t transitionCallback,
    HsmTransitionConditionCallback_t conditionCallback,
    const bool expectedConditionValue) {
    (void)mTransitionsByEvent.emplace(
        std::make_pair(state, onEvent),
        TransitionInfo(state, state, type, transitionCallback, conditionCallback, expectedConditionValue));
}

template <typename HsmStateEnum, typename HsmEventEnum>
HsmStateEnum HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getLastActiveState() const {
    HsmStateEnum currentState = INVALID_HSM_STATE_ID;

    if (false == mActiveStates.empty()) {
        currentState = mActiveStates.back();
    }

    return currentState;
}

template <typename HsmStateEnum, typename HsmEventEnum>
inline const std::list<HsmStateEnum>& HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getActiveStates() const {
    return mActiveStates;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isStateActive(const HsmStateEnum state) const {
    return (std::find(mActiveStates.begin(), mActiveStates.end(), state) != mActiveStates.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionEx(const HsmEventEnum event,
                                                                        const bool clearQueue,
                                                                        const bool sync,
                                                                        const int timeoutMs,
                                                                        Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("transitionEx: event=<%s>, clearQueue=%s, sync=%s",
                              getEventName(event).c_str(),
                              BOOL2STR(clearQueue),
                              BOOL2STR(sync));
    VariantVector_t eventArgs;

    makeVariantList(eventArgs, args...);

    return transitionExWithArgsArray(event, clearQueue, sync, timeoutMs, eventArgs);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transition(const HsmEventEnum event, Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());

    // NOTE: async transitions always return true
    (void)transitionEx(event, false, false, 0, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionWithArgsArray(const HsmEventEnum event,
                                                                                   const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, args.size=%lu", getEventName(event).c_str(), args.size());

    (void)transitionExWithArgsArray(event, false, false, 0, args);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionExWithArgsArray(const HsmEventEnum event,
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

    return status;
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionSync(const HsmEventEnum event,
                                                                          const int timeoutMs,
                                                                          Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());
    return transitionEx(event, false, true, timeoutMs, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionWithQueueClear(const HsmEventEnum event, Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());

    // NOTE: async transitions always return true
    (void)transitionEx(event, true, false, 0, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionInterruptSafe(const HsmEventEnum event) {
    bool res = false;

    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        res = mDispatcher->enqueueEvent(mEnqueuedEventsHandlerId, static_cast<EventID_t>(event));
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTransitionPossible(const HsmEventEnum event, Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());
    bool possible = false;

    for (auto it = mActiveStates.begin(); it != mActiveStates.end(); ++it) {
        possible = checkTransitionPossibility(*it, event, args...);

        if (true == possible) {
            break;
        }
    }

    HSM_TRACE_CALL_RESULT("%d", BOOL2INT(possible));
    return possible;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::startTimer(const TimerID_t timerID,
                                                                      const unsigned int intervalMs,
                                                                      const bool isSingleShot) {
    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->startTimer(mTimerHandlerId, timerID, intervalMs, isSingleShot);
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::restartTimer(const TimerID_t timerID) {
    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->restartTimer(timerID);
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::stopTimer(const TimerID_t timerID) {
    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        mDispatcher->stopTimer(timerID);
    }
}

// ============================================================================
// PRIVATE
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::handleStartup() {
    HSM_TRACE_CALL_DEBUG_ARGS("mActiveStates.size=%ld", mActiveStates.size());

    // NOTE: false-positive. std::shated_ptr has a bool() operator
    // cppcheck-suppress misra-c2012-14.4
    if (mDispatcher) {
        {
            HSM_TRACE_DEBUG("state=<%s>", getStateName(mInitialState).c_str());
            std::list<HsmStateEnum> entryPoints;

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
        }

        if (false == mPendingEvents.empty()) {
            mDispatcher->emitEvent(mEventsHandlerId);
        }
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::makeVariantList(VariantVector_t& vList, Args&&... args) {
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
    (void)make_variant;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchEvents() {
    HSM_TRACE_CALL_DEBUG_ARGS("mPendingEvents.size=%ld", mPendingEvents.size());

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
template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchTimerEvent(const TimerID_t id) {
    HSM_TRACE_CALL_DEBUG_ARGS("id=%d", SC2INT(id));
    auto it = mTimers.find(id);

    if (mTimers.end() != it) {
        transition(it->second);
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateExiting(const HsmStateEnum state) {
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

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateEntering(const HsmStateEnum state,
                                                                           const VariantVector_t& args) {
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

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateChanged(const HsmStateEnum state,
                                                                          const VariantVector_t& args) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>", getStateName(state).c_str());
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onStateChanged) {
        it->second.onStateChanged(args);
        logHsmAction(HsmLogAction::CALLBACK_STATE, INVALID_HSM_STATE_ID, state, INVALID_HSM_EVENT_ID, false, args);
    } else {
        HSM_TRACE_WARNING("no callback registered for state <%s>", getStateName(state).c_str());
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::executeStateAction(const HsmStateEnum state,
                                                                              const StateActionTrigger actionTrigger) {
    HSM_TRACE_CALL_DEBUG_ARGS("state=<%s>, actionTrigger=%d", getStateName(state).c_str(), SC2INT(actionTrigger));
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

                transitionWithArgsArray(static_cast<HsmEventEnum>(actionInfo.actionArgs[0].toInt64()), transitionArgs);
            } else {
                HSM_TRACE_WARNING("unsupported action <%d>", SC2INT(actionInfo.action));
            }
        }
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getParentState(const HsmStateEnum child, HsmStateEnum& outParent) {
    bool wasFound = false;
    auto it = std::find_if(mSubstates.begin(), mSubstates.end(), [child](const std::pair<HsmStateEnum, HsmStateEnum>& item) {
        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return (child == item.second);
    });

    if (mSubstates.end() != it) {
        outParent = it->first;
        wasFound = true;
    }

    return wasFound;
}
template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isSubstateOf(const HsmStateEnum parent, const HsmStateEnum child) {
    HSM_TRACE_CALL_DEBUG_ARGS("parent=<%s>, child=<%s>", getStateName(parent).c_str(), getStateName(child).c_str());
    HsmStateEnum curState = child;

    do {
        if (false == getParentState(curState, curState)) {
            break;
        }
    } while (parent != curState);

    return (parent == curState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getHistoryParent(const HsmStateEnum historyState,
                                                                            HsmStateEnum& outParent) {
    bool wasFound = false;
    auto it = std::find_if(mHistoryStates.begin(),
                           mHistoryStates.end(),
                           [historyState](const std::pair<HsmStateEnum, HsmStateEnum>& item) {
                               // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
                               // cppcheck-suppress misra-c2012-15.5
                               return (historyState == item.second);
                           });

    if (mHistoryStates.end() != it) {
        outParent = it->first;
        wasFound = true;
    }

    return wasFound;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::updateHistory(const HsmStateEnum topLevelState,
                                                                         const std::list<HsmStateEnum>& exitedStates) {
    HSM_TRACE_CALL_DEBUG_ARGS("topLevelState=<%s>, exitedStates.size=%ld",
                              getStateName(topLevelState).c_str(),
                              exitedStates.size());

    std::list<std::list<HsmStateEnum>*> upatedHistory;

    for (auto itActiveState = exitedStates.begin(); itActiveState != exitedStates.end(); ++itActiveState) {
        HsmStateEnum curState = *itActiveState;
        HsmStateEnum parentState;

        while (true == getParentState(curState, parentState)) {
            HSM_TRACE_DEBUG("curState=<%s>, parentState=<%s>",
                            getStateName(curState).c_str(),
                            getStateName(parentState).c_str());
            auto itRange = mHistoryStates.equal_range(parentState);

            if (itRange.first != itRange.second) {
                HSM_TRACE_DEBUG("parent=<%s> has history items", getStateName(parentState).c_str());

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
                                                getStateName(curState).c_str(),
                                                getStateName(it->second).c_str());
                                itCurHistory->second.previousActiveStates.push_back(curState);
                            }
                        } else if (HistoryType::DEEP == itCurHistory->second.type) {
                            if (std::find(itCurHistory->second.previousActiveStates.begin(),
                                          itCurHistory->second.previousActiveStates.end(),
                                          *itActiveState) == itCurHistory->second.previousActiveStates.end()) {
                                HSM_TRACE_DEBUG("DEEP -> store state <%s> in history of parent <%s>",
                                                getStateName(*itActiveState).c_str(),
                                                getStateName(it->second).c_str());
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

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::checkTransitionPossibility(const HsmStateEnum fromState,
                                                                                      const HsmEventEnum event,
                                                                                      Args... args) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>", getEventName(event).c_str());

    HsmStateEnum currentState = fromState;
    std::list<TransitionInfo> possibleTransitions;
    HsmEventEnum nextEvent;
    VariantVector_t transitionArgs;
    bool possible = true;

    makeVariantList(transitionArgs, args...);

    {
        // NOTE: findTransitionTarget can be a bit heavy. possible optimization to reduce lock time is
        //       to make a copy of mPendingEvents and work with it
        HSM_SYNC_EVENTS_QUEUE();

        for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) && (true == possible); ++it) {
            nextEvent = it->type;
            possible = findTransitionTarget(currentState, nextEvent, transitionArgs, true, possibleTransitions);

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
        possible = findTransitionTarget(currentState, nextEvent, transitionArgs, true, possibleTransitions);
    }

    HSM_TRACE_CALL_RESULT("%d", BOOL2INT(possible));
    return possible;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::findTransitionTarget(const HsmStateEnum fromState,
                                                                                const HsmEventEnum event,
                                                                                const VariantVector_t& transitionArgs,
                                                                                const bool searchParents,
                                                                                std::list<TransitionInfo>& outTransitions) {
    HSM_TRACE_CALL_DEBUG_ARGS("fromState=<%s>, event=<%s>", getStateName(fromState).c_str(), getEventName(event).c_str());
    bool continueSearch;
    HsmStateEnum curState = fromState;

    do {
        auto key = std::make_pair(curState, event);
        auto itRange = mTransitionsByEvent.equal_range(key);

        continueSearch = false;

        if (itRange.first == itRange.second) {
            if (true == searchParents) {
                HsmStateEnum parentState;
                bool hasParent = getParentState(curState, parentState);

                if (true == hasParent) {
                    curState = parentState;
                    continueSearch = true;
                }
            }
        } else {
            for (auto it = itRange.first; it != itRange.second; ++it) {
                HSM_TRACE_DEBUG("check transition to <%s>...", getStateName(it->second.destinationState).c_str());

                if ((nullptr == it->second.checkCondition) ||
                    (it->second.expectedConditionValue == it->second.checkCondition(transitionArgs))) {
                    bool wasFound = false;
                    std::list<HsmStateEnum> parentStates = {it->second.destinationState};

                    // cppcheck-suppress misra-c2012-15.4
                    do {
                        HsmStateEnum currentParent = parentStates.front();

                        parentStates.pop_front();

                        // if state has substates we must check if transition into them is possible (after cond)
                        if (true == hasSubstates(currentParent)) {
                            if (true == hasEntryPoint(currentParent)) {
                                HSM_TRACE_DEBUG("state <%s> has entrypoints", getStateName(currentParent).c_str());
                                std::list<HsmStateEnum> entryPoints;

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
        }
    } while (true == continueSearch);

    HSM_TRACE_CALL_RESULT("%s", BOOL2STR(outTransitions.empty() == false));
    return (outTransitions.empty() == false);
}

template <typename HsmStateEnum, typename HsmEventEnum>
typename HsmEventStatus_t HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::doTransition(const PendingEventInfo& event) {
    HSM_TRACE_CALL_DEBUG_ARGS("event=<%s>, transitionType=%d", getEventName(event.type).c_str(), SC2INT(event.transitionType));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
    auto activeStatesSnapshot = mActiveStates;
    std::list<HsmStateEnum> acceptedStates;  // list of states that accepted transitions

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

template <typename HsmStateEnum, typename HsmEventEnum>
typename HsmEventStatus_t
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::handleSingleTransition(const HsmStateEnum activeState,
                                                                             const PendingEventInfo& event) {
    HSM_TRACE_CALL_DEBUG_ARGS("activeState=<%s>, event=<%s>, transitionType=%d",
                              getStateName(activeState).c_str(),
                              getEventName(event.type).c_str(),
                              SC2INT(event.transitionType));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
    const HsmStateEnum fromState = activeState;
    bool isCorrectTransition = false;
    std::list<TransitionInfo> matchingTransitions;

    DEBUG_DUMP_ACTIVE_STATES();

    // ========================================================
    // determine target state based on current transition
    if (TransitionBehavior::REGULAR == event.transitionType) {
        isCorrectTransition = findTransitionTarget(fromState, event.type, event.args, false, matchingTransitions);

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
                HsmStateEnum activeParent = INVALID_HSM_STATE_ID;

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
            std::list<HsmStateEnum> entryStates;

            isCorrectTransition = getEntryPoints(fromState, event.type, event.args, entryStates);

            if (true == isCorrectTransition) {
                for (auto it = entryStates.begin(); it != entryStates.end(); ++it) {
                    (void)matchingTransitions.emplace_back(
                        TransitionInfo{fromState, *it, TransitionType::EXTERNAL_TRANSITION, nullptr, nullptr});
                }
            } else {
                HSM_TRACE_WARNING("state <%s> doesn't have a suitable entry point (event <%s>)",
                                  getStateName(fromState).c_str(),
                                  getEventName(event.type).c_str());
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
        std::list<HsmStateEnum> exitedStates;

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
                                        getStateName(it->fromState).c_str(),
                                        getStateName(*itActiveState).c_str());
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
                        std::list<HsmStateEnum> entryPoints;

                        if (true == replaceActiveState(fromState, it->destinationState)) {
                            onStateChanged(it->destinationState, event.args);
                        }

                        // check if current state is a final state
                        const auto itFinalStateEvent = mFinalStates.find(it->destinationState);

                        if (itFinalStateEvent != mFinalStates.end()) {
                            HsmStateEnum parentState = INVALID_HSM_STATE_ID;

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
                                                getStateName(it->destinationState).c_str(),
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

                                    HsmStateEnum historyParent;

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
                                    std::list<HsmStateEnum> historyTargets;
                                    HsmStateEnum historyParent;

                                    if (true == getHistoryParent(it->destinationState, historyParent)) {
                                        HSM_TRACE_DEBUG("found parent=<%s> for history state=<%s>",
                                                        getStateName(historyParent).c_str(),
                                                        getStateName(it->destinationState).c_str());

                                        if (INVALID_HSM_STATE_ID == itHistoryData->second.defaultTarget) {
                                            // transition to parent's entry point if there is no default history target
                                            historyTargets.push_back(historyParent);
                                        } else {
                                            historyTargets.push_back(itHistoryData->second.defaultTarget);
                                            historyTargets.push_back(historyParent);
                                        }
                                    } else {
                                        HSM_TRACE_ERROR("parent for history state=<%s> wasnt found",
                                                        getStateName(it->destinationState).c_str());
                                    }

                                    PendingEventInfo defHistoryTransitionEvent = event;

                                    defHistoryTransitionEvent.transitionType = TransitionBehavior::FORCED;

                                    for (const HsmStateEnum historyTargetState : historyTargets) {
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
                                                getStateName(it->destinationState).c_str(),
                                                SC2INT(entryPoints.size()),
                                                getStateName(entryPoints.front()).c_str());
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
                        getEventName(event.type).c_str(),
                        getStateName(fromState).c_str());
    }

    DEBUG_DUMP_ACTIVE_STATES();
    HSM_TRACE_CALL_RESULT("%d", SC2INT(res));
    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::clearPendingEvents() {
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
template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::~PendingEventInfo() {
    if (true == cvLock.unique()) {
        HSM_TRACE_CALL_DEBUG_ARGS("event=<%d> was deleted. releasing lock", SC2INT(type));
        unlock(HsmEventStatus_t::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::initLock() {
    if (!cvLock) {
        cvLock = std::make_shared<Mutex>();
        syncProcessed = std::make_shared<ConditionVariable>();
        transitionStatus = std::make_shared<HsmEventStatus_t>();
        *transitionStatus = HsmEventStatus_t::PENDING;
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::releaseLock() {
    if (true == isSync()) {
        HSM_TRACE_CALL_DEBUG_ARGS("releaseLock");
        unlock(HsmEventStatus_t::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::isSync() {
    return (nullptr != cvLock);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::wait(const int timeoutMs) {
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

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::unlock(const HsmEventStatus_t status) {
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

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::hasSubstates(const HsmStateEnum parent) const {
    return (mSubstates.find(parent) != mSubstates.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::hasEntryPoint(const HsmStateEnum state) const {
    return (mSubstateEntryPoints.find(state) != mSubstateEntryPoints.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getEntryPoints(const HsmStateEnum state,
                                                                          const HsmEventEnum onEvent,
                                                                          const VariantVector_t& transitionArgs,
                                                                          std::list<HsmStateEnum>& outEntryPoints) const {
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

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::replaceActiveState(const HsmStateEnum oldState,
                                                                              const HsmStateEnum newState) {
    HSM_TRACE_CALL_DEBUG_ARGS("oldState=<%s>, newState=<%s>", getStateName(oldState).c_str(), getStateName(newState).c_str());

    if (false == isSubstateOf(oldState, newState)) {
        mActiveStates.remove(oldState);
    }

    return addActiveState(newState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::addActiveState(const HsmStateEnum newState) {
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

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTopState(const HsmStateEnum state) const {
    auto it = std::find(mTopLevelStates.begin(), mTopLevelStates.end(), state);

    return (it == mTopLevelStates.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isSubstate(const HsmStateEnum state) const {
    bool result = false;

    for (auto itSubstate = mSubstates.begin(); itSubstate != mSubstates.end(); ++itSubstate) {
        if (itSubstate->second == state) {
            result = true;
            break;
        }
    }

    return result;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::hasParentState(const HsmStateEnum state,
                                                                          HsmStateEnum& outParent) const {
    bool hasParent = false;

    for (auto it = mSubstates.begin(); it != mSubstates.end(); ++it) {
        if (state == it->second) {
            hasParent = true;
            outParent = it->first;
            break;
        }
    }

    return hasParent;
}

#endif  // HSM_ENABLE_SAFE_STRUCTURE

template <typename HsmStateEnum, typename HsmEventEnum>
std::string HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getStateName(const HsmStateEnum state) const {
    std::string name;

    if (state != INVALID_HSM_STATE_ID) {
        name = std::to_string(static_cast<int>(state));
    }

    return name;
}

template <typename HsmStateEnum, typename HsmEventEnum>
std::string HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getEventName(const HsmEventEnum event) const {
    std::string name;

    if (event != INVALID_HSM_EVENT_ID) {
        name = std::to_string(static_cast<int>(event));
    }

    return name;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::enableHsmDebugging() {
#ifdef HSMBUILD_DEBUGGING
    char* envPath = std::getenv(ENV_DUMPPATH);

    return enableHsmDebugging((nullptr == envPath) ? DEFAULT_DUMP_PATH : std::string(envPath));
#else
    return true;
#endif
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::enableHsmDebugging(const std::string& dumpPath) {
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

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::disableHsmDebugging() {
#ifdef HSMBUILD_DEBUGGING
    mHsmLogFile.close();
#endif
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::logHsmAction(const HsmLogAction action,
                                                                        const HsmStateEnum fromState,
                                                                        const HsmStateEnum targetState,
                                                                        const HsmEventEnum event,
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
            *mHsmLog << "\n    - \"" << getStateName(*itState) << "\"";
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

        for (auto itArg = args.begin(); itArg != args.end(); ++itArg) {
            *mHsmLog << "\n    - " << itArg->toString();
        }

        mHsmLog->flush();
    }
#endif  // HSMBUILD_DEBUGGING
}

}  // namespace hsmcpp

#endif  // HSMCPP_HSM_HPP
