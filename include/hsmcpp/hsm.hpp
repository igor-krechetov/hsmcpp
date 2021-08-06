// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSM_HPP__
#define __HSMCPP_HSM_HPP__

#include <fstream>
#include <ctime>
#include <algorithm>
#include <functional>
#include <atomic>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "IHsmEventDispatcher.hpp"
#include "variant.hpp"
#include "logging.hpp"

#ifdef HSMBUILD_DEBUGGING
  #include <cstdlib>
  #include <cstring>
#endif

// WIN, access
#ifdef WIN32
  #include <io.h>
  #define F_OK            0
#else
  #include <unistd.h>
  #include <time.h>
#endif

namespace hsmcpp
{
// If defined, HSM will performe safety checks during states and substates registration.
// Normally HSM structure should be static, so this feature is usefull only
// during development since it reduces performance a bit
// #define HSM_ENABLE_SAFE_STRUCTURE                    1

// Thread safety is enabled by default, but it adds some overhead related with mutex usage.
// If performance is critical and it's ensured that HSM is used only from a single thread,
// then synchronization could be disabled during compilation.
// #define HSM_DISABLE_THREADSAFETY                     1

#ifdef HSM_DISABLE_THREADSAFETY
  #define _HSM_SYNC_EVENTS_QUEUE()
#else
  #define _HSM_SYNC_EVENTS_QUEUE()                std::lock_guard<std::mutex> lck(mEventsSync)
#endif // HSM_DISABLE_THREADSAFETY

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HierarchicalStateMachine"

#define HSM_WAIT_INDEFINITELY                   (0)
#define INVALID_HSM_EVENT_ID                    static_cast<HsmEventEnum>(-1000)
#define INVALID_HSM_STATE_ID                    static_cast<HsmStateEnum>(-1000)

#define ENV_DUMPPATH                            "HSMCPP_DUMP_PATH"
#define DEFAULT_DUMP_PATH                       "./dump.hsmlog"

typedef std::vector<Variant> VariantList_t;

template <typename HsmStateEnum, typename HsmEventEnum>
class HierarchicalStateMachine
{
public:
    typedef std::function<void(const VariantList_t&)> HsmTransitionCallback_t;
    typedef std::function<bool(const VariantList_t&)> HsmTransitionConditionCallback_t;
    typedef std::function<void(const VariantList_t&)> HsmStateChangedCallback_t;
    typedef std::function<bool(const VariantList_t&)> HsmStateEnterCallback_t;
    typedef std::function<bool(void)>                 HsmStateExitCallback_t;

    #define HsmTransitionCallbackPtr_t(_class, _func)              void (_class::*_func)(const VariantList_t&)
    #define HsmTransitionConditionCallbackPtr_t(_class, _func)     bool (_class::*_func)(const VariantList_t&)
    #define HsmStateChangedCallbackPtr_t(_class, _func)            void (_class::*_func)(const VariantList_t&)
    #define HsmStateEnterCallbackPtr_t(_class, _func)              bool (_class::*_func)(const VariantList_t&)
    #define HsmStateExitCallbackPtr_t(_class, _func)               bool (_class::*_func)()

    enum class HistoryType
    {
        SHALLOW,
        DEEP
    };

private:
    enum class HsmLogAction
    {
        IDLE,
        TRANSITION,
        TRANSITION_ENTRYPOINT,
        CALLBACK_EXIT,
        CALLBACK_ENTER,
        CALLBACK_STATE
    };

    enum class HsmEventStatus
    {
        PENDING,
        DONE_OK,
        DONE_FAILED,
        CANCELED
    };

    enum class TransitionType
    {
        REGULAR,
        ENTRYPOINT,
        FORCED
    };

    // NOTE: just an alias to make code more readable
#ifdef WIN32
    #define HsmEventStatus_t   typename HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HsmEventStatus
#else
    #define HsmEventStatus_t   HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HsmEventStatus
#endif

    struct StateCallbacks
    {
        HsmStateChangedCallback_t onStateChanged = nullptr;
        HsmStateEnterCallback_t onEntering = nullptr;
        HsmStateExitCallback_t onExiting = nullptr;
    };

    struct StateEntryPoint
    {
        HsmStateEnum state;
        HsmEventEnum onEvent;
    };

    struct TransitionInfo
    {
        HsmStateEnum fromState = INVALID_HSM_STATE_ID;
        HsmStateEnum destinationState = INVALID_HSM_STATE_ID;
        HsmTransitionCallback_t onTransition = nullptr;
        HsmTransitionConditionCallback_t checkCondition = nullptr;

        TransitionInfo()
           : fromState(INVALID_HSM_STATE_ID)
           , destinationState(INVALID_HSM_STATE_ID)
           , onTransition(nullptr)
           , checkCondition(nullptr)
        {}

        TransitionInfo(const HsmStateEnum from,
                       const HsmStateEnum to,
                       HsmTransitionCallback_t cbTransition,
                       HsmTransitionConditionCallback_t cbCondition)
           : fromState(from)
           , destinationState(to)
           , onTransition(cbTransition)
           , checkCondition(cbCondition)
        {}
    };

    struct PendingEventInfo
    {
        TransitionType transitionType = TransitionType::REGULAR;
        HsmEventEnum type = INVALID_HSM_EVENT_ID;
        VariantList_t args;
        std::shared_ptr<std::mutex> cvLock;
        std::shared_ptr<std::condition_variable> syncProcessed;
        std::shared_ptr<HsmEventStatus> transitionStatus;
        std::shared_ptr<std::list<TransitionInfo>> forcedTransitionsInfo;

        ~PendingEventInfo();
        void initLock();
        void releaseLock();
        bool isSync();
        void wait(const int timeoutMs = HSM_WAIT_INDEFINITELY);
        void unlock(const HsmEventStatus status);
    };

    struct HistoryInfo
    {
        HistoryType type = HistoryType::SHALLOW;
        HsmStateEnum defaultTarget = INVALID_HSM_STATE_ID;
        HsmTransitionCallback_t transitionCallback = nullptr;
        std::set<HsmStateEnum> previousActiveStates;

        HistoryInfo(const HistoryType newType,
                    const HsmStateEnum newDefaultTarget,
                    HsmTransitionCallback_t newTransitionCallback)
            : type(newType)
            , defaultTarget(newDefaultTarget)
            , transitionCallback(newTransitionCallback)
        {}
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
    bool initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher);

    // Releases dispatcher and resets all internal resources. HSM cant be reused after calling this API.
    // Must be called on the same thread as initialize()
    //
    // NOTE: Usually you dont need to call this function directly. The only scenario when it's needed is
    //       for multithreaded environment where it's impossible to delete HSM on the same thread where it was initialized.
    //       Then you must call release() on the Dispatcher's thread before deleting HSM instance on another thread.
    void release();

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
    bool registerSubstateEntryPoint(const HsmStateEnum parent, const HsmStateEnum substate, const HsmEventEnum onEvent = INVALID_HSM_EVENT_ID);

    template <class HsmHandlerClass>
    void registerTransition(const HsmStateEnum from,
                            const HsmStateEnum to,
                            const HsmEventEnum onEvent,
                            HsmHandlerClass* handler = nullptr,
                            HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback) = nullptr,
                            HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback) = nullptr);

    void registerTransition(const HsmStateEnum from,
                            const HsmStateEnum to,
                            const HsmEventEnum onEvent,
                            HsmTransitionCallback_t transitionCallback = nullptr,
                            HsmTransitionConditionCallback_t conditionCallback = nullptr);

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

    // sync transition
    template <typename... Args>
    bool transitionSync(const HsmEventEnum event, const int timeoutMs, Args... args);

    // async transition which clears events queue before adding requested event
    template <typename... Args>
    void transitionWithQueueClear(const HsmEventEnum event, Args... args);

    template <typename... Args>
    bool isTransitionPossible(const HsmEventEnum event, Args... args);

    // By default log will be written to ./dump.hsmlog file.
    // This location can be overwritten by setting ENV_DUMPPATH environment variable with desired path.
    bool enableHsmDebugging();
    bool enableHsmDebugging(const std::string& dumpPath);
    void disableHsmDebugging();

private:
    // checks initial state and, if needed, process any automatic initial transitions
    void handleStartup();

    template <typename... Args>
    void makeVariantList(VariantList_t& vList, Args&&... args);

    bool registerSubstate(const HsmStateEnum parent,
                          const HsmStateEnum substate,
                          const bool isEntryPoint,
                          const HsmEventEnum eventCondition = INVALID_HSM_EVENT_ID);

    void dispatchEvents();

    bool onStateExiting(const HsmStateEnum state);
    bool onStateEntering(const HsmStateEnum state, const VariantList_t& args);
    void onStateChanged(const HsmStateEnum state, const VariantList_t& args);

    bool getParentState(const HsmStateEnum child, HsmStateEnum& outParent);
    bool isSubstateOf(const HsmStateEnum parent, const HsmStateEnum child);

    bool getHistoryParent(const HsmStateEnum historyState, HsmStateEnum& outParent);
    void updateHistory(const HsmStateEnum topLevelState, const std::list<HsmStateEnum>& activeStates);

    template <typename... Args>
    bool isTransitionPossible(const HsmStateEnum fromState, const HsmEventEnum event, Args... args);

    bool findTransitionTarget(const HsmStateEnum fromState,
                              const HsmEventEnum event,
                              const VariantList_t& transitionArgs,
                              std::list<TransitionInfo>& outTransitions);
    HsmEventStatus doTransition(const PendingEventInfo& event);
    HsmEventStatus handleSingleTransition(const HsmStateEnum fromState, const PendingEventInfo& event);
    void clearPendingEvents();

    bool hasSubstates(const HsmStateEnum parent) const;
    bool hasEntryPoint(const HsmStateEnum state) const;
    // TODO: return enum instead of bool (no entrypoint registered, no matching entry, ok)
    bool getEntryPoints(const HsmStateEnum state, const HsmEventEnum onEvent, std::list<HsmStateEnum>& outEntryPoints) const;

    // returns TRUE if newState was added to a list of active states
    bool replaceActiveState(const HsmStateEnum oldState, const HsmStateEnum newState);
    // returns TRUE if newState was added to a list of active states
    bool addActiveState(const HsmStateEnum newState);

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    bool isTopState(const HsmStateEnum state) const;
    bool isSubstate(const HsmStateEnum state) const;
    bool hasParentState(const HsmStateEnum state, HsmStateEnum &outParent) const;
#endif // HSM_ENABLE_SAFE_STRUCTURE

    void logHsmAction(const HsmLogAction action,
                      const HsmStateEnum fromState = INVALID_HSM_STATE_ID,
                      const HsmStateEnum targetState = INVALID_HSM_STATE_ID,
                      const HsmEventEnum event = INVALID_HSM_EVENT_ID,
                      const bool hasFailed = false,
                      const VariantList_t& args = VariantList_t());

protected:
    // NOTE: clients must implement this method for debugging to work. names should match with the names in scxml file
    virtual std::string getStateName(const HsmStateEnum state);
    virtual std::string getEventName(const HsmEventEnum event);

private:
    HsmStateEnum mInitialState;
    std::list<HsmStateEnum> mActiveStates;
    std::multimap<std::pair<HsmStateEnum, HsmEventEnum>, TransitionInfo> mTransitionsByEvent; // FROM_STATE, EVENT => TO
    std::map<HsmStateEnum, StateCallbacks> mRegisteredStates;
    std::multimap<HsmStateEnum, HsmStateEnum> mSubstates;
    std::multimap<HsmStateEnum, StateEntryPoint> mSubstateEntryPoints;
    std::list<PendingEventInfo> mPendingEvents;
    std::shared_ptr<IHsmEventDispatcher> mDispatcher;

    // parent state, history state
    std::multimap<HsmStateEnum, HsmStateEnum> mHistoryStates;
    // history state id, data
    std::map<HsmStateEnum, HistoryInfo> mHistoryData;

    int mDispatcherHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    bool mStopDispatching = false;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    std::list<HsmStateEnum> mTopLevelStates; // list of states which are not substates and dont have substates of their own
#endif // HSM_ENABLE_SAFE_STRUCTURE

#ifndef HSM_DISABLE_THREADSAFETY
    std::mutex mEventsSync;
#endif// HSM_DISABLE_THREADSAFETY

#ifdef HSMBUILD_DEBUGGING
    std::filebuf mHsmLogFile;
    std::shared_ptr<std::ostream> mHsmLog;
#endif // HSMBUILD_DEBUGGING
};

// ============================================================================
// PUBLIC
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HierarchicalStateMachine(const HsmStateEnum initialState)
    : mInitialState(initialState)
{
    __HSM_TRACE_INIT__();
    mActiveStates.push_back(mInitialState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::~HierarchicalStateMachine()
{
    release();
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::setInitialState(const HsmStateEnum initialState)
{
    if (!mDispatcher)
    {
        mActiveStates.clear();
        mInitialState = initialState;
        mActiveStates.push_back(initialState);
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher)
{
    __HSM_TRACE_CALL_DEBUG__();
    bool result = false;

    if (!mDispatcher)
    {
        if (dispatcher)
        {
            if (true == dispatcher->start())
            {
                mDispatcher = dispatcher;
                mDispatcherHandlerId = mDispatcher->registerEventHandler(std::bind(&HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchEvents,
                                                                        this));

                result = (INVALID_HSM_DISPATCHER_HANDLER_ID != mDispatcherHandlerId);

                if (true == result)
                {
                    logHsmAction(HsmLogAction::IDLE, INVALID_HSM_STATE_ID, INVALID_HSM_STATE_ID, INVALID_HSM_EVENT_ID, false, VariantList_t());
                    handleStartup();
                }
            }
            else
            {
                __HSM_TRACE_ERROR__("failed to start dispatcher");
            }
        }
        else
        {
            __HSM_TRACE_ERROR__("dispatcher is NULL");
        }
    }
    else
    {
        __HSM_TRACE_ERROR__("already initialized");
    }

    return result;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::release()
{
    mStopDispatching = true;
    __HSM_TRACE_CALL_DEBUG__();

    disableHsmDebugging();

    if (mDispatcher)
    {
        mDispatcher->unregisterEventHandler(mDispatcherHandlerId);
        mDispatcher.reset();
        mDispatcherHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerState(const HsmStateEnum state,
                                                                         HsmHandlerClass* handler,
                                                                         HsmStateChangedCallbackPtr_t(HsmHandlerClass, onStateChanged),
                                                                         HsmStateEnterCallbackPtr_t(HsmHandlerClass, onEntering),
                                                                         HsmStateExitCallbackPtr_t(HsmHandlerClass, onExiting))
{
    HsmStateChangedCallback_t funcStateChanged;
    HsmStateEnterCallback_t funcEntering;
    HsmStateExitCallback_t funcExiting;

    if (nullptr != handler)
    {
        if (nullptr != onStateChanged)
        {
            funcStateChanged = std::bind(onStateChanged, handler, std::placeholders::_1);
        }

        if (nullptr != onEntering)
        {
            funcEntering = std::bind(onEntering, handler, std::placeholders::_1);
        }

        if (nullptr != onExiting)
        {
            funcExiting = std::bind(onExiting, handler);
        }
    }

    registerState(state, funcStateChanged, funcEntering, funcExiting);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerState(const HsmStateEnum state,
                                                                         HsmStateChangedCallback_t onStateChanged,
                                                                         HsmStateEnterCallback_t onEntering,
                                                                         HsmStateExitCallback_t onExiting)
{
#ifdef HSM_ENABLE_SAFE_STRUCTURE
    if ((false == isSubstate(state)) && (false == isTopState(state)))
    {
        mTopLevelStates.push_back(state);
    }
#endif // HSM_ENABLE_SAFE_STRUCTURE

    if (onStateChanged || onEntering || onExiting)
    {
        StateCallbacks cb;

        cb.onStateChanged = onStateChanged;
        cb.onEntering = onEntering;
        cb.onExiting = onExiting;
        mRegisteredStates[state] = cb;

        __HSM_TRACE_CALL_DEBUG_ARGS__("mRegisteredStates.size=%ld", mRegisteredStates.size());
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerHistory(const HsmStateEnum parent,
                                                                           const HsmStateEnum historyState,
                                                                           const HistoryType type,
                                                                           const HsmStateEnum defaultTarget,
                                                                           HsmHandlerClass* handler,
                                                                           HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback))
{
    HsmTransitionCallback_t funcTransitionCallback;

    if (nullptr != handler)
    {
        if (nullptr != transitionCallback)
        {
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
                                                                           HsmTransitionCallback_t transitionCallback)
{
    mHistoryStates.emplace(parent, historyState);
    mHistoryData.emplace(historyState, HistoryInfo(type, defaultTarget, transitionCallback));
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstate(const HsmStateEnum parent,
                                                                            const HsmStateEnum substate)
{
    return registerSubstate(parent, substate, false);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstateEntryPoint(const HsmStateEnum parent,
                                                                                      const HsmStateEnum substate,
                                                                                      const HsmEventEnum onEvent)
{
    return registerSubstate(parent, substate, true, onEvent);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerSubstate(const HsmStateEnum parent,
                                                                            const HsmStateEnum substate,
                                                                            const bool isEntryPoint,
                                                                            const HsmEventEnum onEvent)
{
    bool registrationAllowed = false;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    // do a simple sanity check
    if (parent != substate)
    {
        HsmStateEnum curState = parent;
        HsmStateEnum prevState;

        if (false == hasParentState(substate, prevState))
        {
            registrationAllowed = true;

            while (true == hasParentState(curState, prevState))
            {
                if (substate == prevState)
                {
                    __HSM_TRACE_CALL_DEBUG_ARGS__("requested operation will result in substates recursion (parent=%d, substate=%d)",
                                              SC2INT(parent), SC2INT(substate));
                    registrationAllowed = false;
                    break;
                }

                curState = prevState;
            }
        }
        else
        {
            __HSM_TRACE_CALL_DEBUG_ARGS__("substate %d already has a parent %d",
                                      SC2INT(substate), SC2INT(prevState));
        }
    }
#else
    registrationAllowed = (parent != substate);
#endif // HSM_ENABLE_SAFE_STRUCTURE

    if (registrationAllowed)
    {
        if (isEntryPoint)
        {
            StateEntryPoint entryInfo = {substate, onEvent};

            mSubstateEntryPoints.emplace(parent, entryInfo);
        }

        mSubstates.emplace(parent, substate);

#ifdef HSM_ENABLE_SAFE_STRUCTURE
        if (isTopState(substate))
        {
            mTopLevelStates.remove(substate);
        }
#endif // HSM_ENABLE_SAFE_STRUCTURE
    }

    return registrationAllowed;
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerTransition(const HsmStateEnum from,
                                                                              const HsmStateEnum to,
                                                                              const HsmEventEnum onEvent,
                                                                              HsmHandlerClass* handler,
                                                                              HsmTransitionCallbackPtr_t(HsmHandlerClass, transitionCallback),
                                                                              HsmTransitionConditionCallbackPtr_t(HsmHandlerClass, conditionCallback))
{
    HsmTransitionCallback_t funcTransitionCallback;
    HsmTransitionConditionCallback_t funcConditionCallback;

    if (nullptr != handler)
    {
        if (nullptr != transitionCallback)
        {
            funcTransitionCallback = std::bind(transitionCallback, handler, std::placeholders::_1);
        }

        if (nullptr != conditionCallback)
        {
            funcConditionCallback = std::bind(conditionCallback, handler, std::placeholders::_1);
        }
    }

    registerTransition(from, to, onEvent, funcTransitionCallback, funcConditionCallback);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::registerTransition(const HsmStateEnum from,
                                                                              const HsmStateEnum to,
                                                                              const HsmEventEnum onEvent,
                                                                              HsmTransitionCallback_t transitionCallback,
                                                                              HsmTransitionConditionCallback_t conditionCallback)
{
    mTransitionsByEvent.emplace(std::make_pair(from, onEvent), TransitionInfo(from, to, transitionCallback, conditionCallback));
}

template <typename HsmStateEnum, typename HsmEventEnum>
HsmStateEnum HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getLastActiveState() const
{
    HsmStateEnum currentState = INVALID_HSM_STATE_ID;

    if (mActiveStates.size() > 0)
    {
        currentState = mActiveStates.back();
    }

    return currentState;
}

template <typename HsmStateEnum, typename HsmEventEnum>
inline const std::list<HsmStateEnum>& HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getActiveStates() const
{
    return mActiveStates;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isStateActive(const HsmStateEnum state) const
{
    return (std::find(mActiveStates.begin(), mActiveStates.end(), state) != mActiveStates.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionEx(const HsmEventEnum event,
                                                                        const bool clearQueue,
                                                                        const bool sync,
                                                                        const int timeoutMs,
                                                                        Args... args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("transitionEx: event=%d, clearQueue=%s, sync=%s", SC2INT(event), BOOL2STR(clearQueue), BOOL2STR(sync));

    bool status = false;
    PendingEventInfo eventInfo;

    eventInfo.type = event;
    makeVariantList(eventInfo.args, args...);

    if (true == sync)
    {
        eventInfo.initLock();
    }

    {
        _HSM_SYNC_EVENTS_QUEUE();

        if (true == clearQueue)
        {
            clearPendingEvents();
        }

        mPendingEvents.push_back(eventInfo);
    }

    __HSM_TRACE_DEBUG__("transitionEx: emit");
    mDispatcher->emitEvent();

    if (true == sync)
    {
        __HSM_TRACE_DEBUG__("transitionEx: wait...");
        eventInfo.wait(timeoutMs);
        status = (HsmEventStatus_t::DONE_OK == *eventInfo.transitionStatus);
    }
    else
    {
        // always return true for async transitions
        status = true;
    }

    return status;
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transition(const HsmEventEnum event, Args... args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));

    transitionEx(event, false, false, 0, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionSync(const HsmEventEnum event, const int timeoutMs, Args... args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));
    return transitionEx(event, false, true, timeoutMs, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionWithQueueClear(const HsmEventEnum event,
                                                                                    Args... args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));

    transitionEx(event, true, false, 0, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTransitionPossible(const HsmEventEnum event,
                                                                                Args... args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));
    bool possible = false;

    for (auto it = mActiveStates.begin() ; it != mActiveStates.end() ; ++it)
    {
        possible = isTransitionPossible(*it, event, args...);

        if (true == possible)
        {
            break;
        }
    }

    __HSM_TRACE_CALL_RESULT__("%d", BOOL2INT(possible));
    return possible;
}

// ============================================================================
// PRIVATE
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::handleStartup()
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("mActiveStates.size=%ld", mActiveStates.size());

    if (mDispatcher)
    {
        {
            std::list<HsmStateEnum> entryPoints;

            _HSM_SYNC_EVENTS_QUEUE();

            for (auto it = mActiveStates.begin(); it != mActiveStates.end(); ++it)
            {
                __HSM_TRACE_DEBUG__("state=%d", SC2INT(*it));

                if (true == getEntryPoints(*it, INVALID_HSM_EVENT_ID, entryPoints))
                {
                    PendingEventInfo entryPointTransitionEvent;

                    entryPointTransitionEvent.transitionType = TransitionType::ENTRYPOINT;
                    entryPointTransitionEvent.type = INVALID_HSM_EVENT_ID;

                    mPendingEvents.push_front(entryPointTransitionEvent);
                }
            }
        }

        if (mPendingEvents.size() > 0)
        {
            mDispatcher->emitEvent();
        }
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::makeVariantList(VariantList_t& vList, Args&&... args)
{
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
    (void)make_variant;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchEvents()
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("dispatchEvents: mPendingEvents.size()=%ld", mPendingEvents.size());

    if (false == mStopDispatching)
    {
        if (false == mPendingEvents.empty())
        {
            PendingEventInfo pendingEvent;

            {
                _HSM_SYNC_EVENTS_QUEUE();
                pendingEvent = mPendingEvents.front();
                mPendingEvents.pop_front();
            }

            HsmEventStatus_t transitiontStatus = doTransition(pendingEvent);

            __HSM_TRACE_DEBUG__("dispatchEvents: unlock with status %d", SC2INT(transitiontStatus));
            pendingEvent.unlock(transitiontStatus);
        }

        if ((false == mStopDispatching) && (false == mPendingEvents.empty()))
        {
            mDispatcher->emitEvent();
        }
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateExiting(const HsmStateEnum state)
{
    bool res = true;
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onExiting)
    {
        res = it->second.onExiting();
        logHsmAction(HsmLogAction::CALLBACK_EXIT, state, INVALID_HSM_STATE_ID, INVALID_HSM_EVENT_ID, (false == res), VariantList_t());
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateEntering(const HsmStateEnum state,
                                                                           const VariantList_t& args)
{
    bool res = true;

    // since we can have a situation when same state is entered twice (parallel transitions) there
    // is no need to call callbacks multiple times
    if (false == isStateActive(state))
    {
        auto it = mRegisteredStates.find(state);

        if ((mRegisteredStates.end() != it) && it->second.onEntering)
        {
            res = it->second.onEntering(args);
            logHsmAction(HsmLogAction::CALLBACK_ENTER, INVALID_HSM_STATE_ID, state, INVALID_HSM_EVENT_ID, (false == res), args);
        }
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateChanged(const HsmStateEnum state,
                                                                          const VariantList_t& args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("state=%d", SC2INT(state));
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onStateChanged)
    {
        it->second.onStateChanged(args);
        logHsmAction(HsmLogAction::CALLBACK_STATE, INVALID_HSM_STATE_ID, state, INVALID_HSM_EVENT_ID, false, args);
    }
    else
    {
        __HSM_TRACE_WARNING__("no callback registered for state <%d>", SC2INT(state));
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getParentState(const HsmStateEnum child,
                                                                          HsmStateEnum& outParent)
{
    bool wasFound = false;
    auto it = std::find_if(mSubstates.begin(), mSubstates.end(),
                          [child](const auto& itemIt){ return (child == itemIt.second); });

    if (mSubstates.end() != it)
    {
        outParent = it->first;
        wasFound = true;
    }

    return wasFound;
}
template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isSubstateOf(const HsmStateEnum parent, const HsmStateEnum child)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("parent=%d, child=%d", SC2INT(parent), SC2INT(child));
    HsmStateEnum curState = child;

    do
    {
        if (false == getParentState(curState, curState))
        {
            break;
        }
    } while (parent != curState);

    return (parent == curState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getHistoryParent(const HsmStateEnum historyState, HsmStateEnum& outParent)
{
    bool wasFound = false;
    auto it = std::find_if(mHistoryStates.begin(), mHistoryStates.end(),
                          [historyState](const auto& itemIt){ return (historyState == itemIt.second); });

    if (mHistoryStates.end() != it)
    {
        outParent = it->first;
        wasFound = true;
    }

    return wasFound;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::updateHistory(const HsmStateEnum topLevelState,
                                                                         const std::list<HsmStateEnum>& activeStates)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("topLevelState=%d, activeStates.size=%ld", SC2INT(topLevelState), activeStates.size());

    for (auto itActiveState = activeStates.begin(); itActiveState != activeStates.end(); ++itActiveState)
    {
        HsmStateEnum curState = *itActiveState;
        HsmStateEnum parentState;

        while (true == getParentState(curState, parentState))
        {
            __HSM_TRACE_DEBUG__("curState=%d, parentState=%d", SC2INT(curState), SC2INT(parentState));
            auto itRange = mHistoryStates.equal_range(parentState);

            if (itRange.first != itRange.second)
            {
                __HSM_TRACE_DEBUG__("parent=%d has history items", SC2INT(parentState));

                for (auto it = itRange.first; it != itRange.second; ++it)
                {
                    auto itCurHistory = mHistoryData.find(it->second);

                    if (itCurHistory != mHistoryData.end())
                    {
                        if (HistoryType::SHALLOW == itCurHistory->second.type)
                        {
                            __HSM_TRACE_DEBUG__("SHALLOW -> store state %d in history of parent %d",
                                            SC2INT(curState), SC2INT(it->second));
                            itCurHistory->second.previousActiveStates.insert(curState);
                        }
                        else if (HistoryType::DEEP == itCurHistory->second.type)
                        {
                            __HSM_TRACE_DEBUG__("DEEP -> store state %d in history of parent %d",
                                            SC2INT(*itActiveState), SC2INT(it->second));
                            itCurHistory->second.previousActiveStates.insert(*itActiveState);
                        }
                    }
                }
            }

            if (topLevelState != parentState)
            {
                curState = parentState;
            }
            else
            {
                break;
            }
        }
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTransitionPossible(const HsmStateEnum fromState,
                                                                                const HsmEventEnum event,
                                                                                Args... args)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));

    HsmStateEnum currentState = fromState;
    std::list<TransitionInfo> possibleTransitions;
    HsmEventEnum nextEvent;
    VariantList_t transitionArgs;
    bool possible = true;

    makeVariantList(transitionArgs, args...);

    {
        _HSM_SYNC_EVENTS_QUEUE();

        for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) && (true == possible); ++it)
        {
            nextEvent = it->type;
            possible = findTransitionTarget(currentState, nextEvent, transitionArgs, possibleTransitions);

            if (true == possible)
            {
                if (possibleTransitions.size() > 0)
                {
                    currentState = possibleTransitions.front().destinationState;
                }
                else
                {
                    possible = false;
                    break;
                }
            }
        }
    }

    if (true == possible)
    {
        nextEvent = event;
        possible = findTransitionTarget(currentState, nextEvent, transitionArgs, possibleTransitions);
    }

    __HSM_TRACE_CALL_RESULT__("%d", BOOL2INT(possible));
    return possible;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::findTransitionTarget(const HsmStateEnum fromState,
                                                                                const HsmEventEnum event,
                                                                                const VariantList_t& transitionArgs,
                                                                                std::list<TransitionInfo>& outTransitions)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("fromState=%d, event=%d", SC2INT(fromState), SC2INT(event));
    bool continueSearch;
    HsmStateEnum curState = fromState;

    do
    {
        auto key = std::make_pair(curState, event);
        auto itRange = mTransitionsByEvent.equal_range(key);

        continueSearch = false;

        if (itRange.first == itRange.second)
        {
            HsmStateEnum parentState;
            bool hasParent = getParentState(curState, parentState);

            if (true == hasParent)
            {
                curState = parentState;
                continueSearch = true;
            }
        }
        else
        {
            for (auto it = itRange.first; it != itRange.second; ++it)
            {
                __HSM_TRACE_DEBUG__("check transition to %d...", SC2INT(it->second.destinationState));

                if ((nullptr == it->second.checkCondition) || (true == it->second.checkCondition(transitionArgs)))
                {
                    bool wasFound = false;
                    std::list<HsmStateEnum> parentStates = {it->second.destinationState};

                    do
                    {
                        HsmStateEnum currentParent = parentStates.front();

                        parentStates.pop_front();

                        // if state has substates we must check if transition into them is possible (after cond)
                        if (true == hasSubstates(currentParent))
                        {
                            if (true == hasEntryPoint(currentParent))
                            {
                                __HSM_TRACE_DEBUG__("state <%d> has entrypoints", SC2INT(currentParent));
                                std::list<HsmStateEnum> entryPoints;

                                if (true == getEntryPoints(currentParent, event, entryPoints))
                                {
                                    parentStates.splice(parentStates.end(), entryPoints);
                                }
                                else
                                {
                                    __HSM_TRACE_WARNING__("no matching entrypoints found");
                                    break;
                                }
                            }
                            else
                            {
                                __HSM_TRACE_WARNING__("state <%d> doesn't have an entrypoint defined", SC2INT(currentParent));
                                break;
                            }
                        }
                        else
                        {
                            outTransitions.push_back(it->second);
                            wasFound = true;
                        }
                    } while((false == wasFound) && (parentStates.empty() == false));
                }
            }
        }
    } while (true == continueSearch);

    __HSM_TRACE_CALL_RESULT__("%s", BOOL2STR(outTransitions.empty() == false));
    return (outTransitions.empty() == false);
}

template <typename HsmStateEnum, typename HsmEventEnum>
typename HsmEventStatus_t HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::doTransition(const PendingEventInfo& event)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d, transitionType=%d", SC2INT(event.type), SC2INT(event.transitionType));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
    auto activeStatesSnapshot = mActiveStates;

    for (auto it = activeStatesSnapshot.begin() ; it != activeStatesSnapshot.end() ; ++it)
    {
        // in case of parallel transitions some states might become inactive after handleSingleTransition()
        // example: [*B, *C] -> D
        if (true == isStateActive(*it))
        {
            const HsmEventStatus_t singleTransitionResult = handleSingleTransition(*it, event);

            switch(singleTransitionResult)
            {
                case HsmEventStatus_t::PENDING:
                    res = singleTransitionResult;
                    break;
                case HsmEventStatus_t::DONE_OK:
                    logHsmAction(HsmLogAction::IDLE, INVALID_HSM_STATE_ID, INVALID_HSM_STATE_ID, INVALID_HSM_EVENT_ID, false, VariantList_t());
                    if (HsmEventStatus_t::PENDING != res)
                    {
                        res = singleTransitionResult;
                    }
                    break;
                case HsmEventStatus_t::CANCELED:
                    break;
                case HsmEventStatus_t::DONE_FAILED:
                default:
                    // do nothing
                    break;
            }
        }
    }

    __HSM_TRACE_CALL_RESULT__("%d", SC2INT(res));
    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
typename HsmEventStatus_t HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::handleSingleTransition(const HsmStateEnum activeState,
                                                                                                       const PendingEventInfo& event)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("activeState=%d, event=%d, transitionType=%d",
                              SC2INT(activeState), SC2INT(event.type), SC2INT(event.transitionType));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
    const HsmStateEnum fromState = activeState;
    bool isCorrectTransition = false;
    std::list<TransitionInfo> matchingTransitions;

    if (TransitionType::REGULAR == event.transitionType)
    {
        isCorrectTransition = findTransitionTarget(fromState, event.type, event.args, matchingTransitions);

        if (false == isCorrectTransition)
        {
            __HSM_TRACE_WARNING__("no suitable transition from state <%d> with event <%d>",
                              SC2INT(fromState), SC2INT(event.type));
        }
    }
    else if (TransitionType::ENTRYPOINT == event.transitionType)
    {
        std::list<HsmStateEnum> entryStates;

        isCorrectTransition = getEntryPoints(fromState, event.type, entryStates);

        if (true == isCorrectTransition)
        {
            for (auto it = entryStates.begin(); it != entryStates.end(); ++it)
            {
                matchingTransitions.emplace_back(TransitionInfo{fromState, *it, nullptr, nullptr});
            }
        }
        else
        {
            __HSM_TRACE_WARNING__("state <%d> doesn't have a suitable entry point (event <%d>)",
                              SC2INT(fromState), SC2INT(event.type));
        }
    }
    else if (TransitionType::FORCED == event.transitionType)
    {
        __HSM_TRACE_DEBUG__("forced history transitions: %d", SC2INT(event.forcedTransitionsInfo->size()));
        matchingTransitions = *event.forcedTransitionsInfo;
        isCorrectTransition = true;
    }

    if (true == isCorrectTransition)
    {
        bool isExitAllowed = true;
        std::list<HsmStateEnum> exitedStates;

        // execute self transitions first
        for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it)
        {
            if (it->fromState == it->destinationState)
            {
                // TODO: separate type for self transition?
                logHsmAction(HsmLogAction::TRANSITION, it->fromState, it->destinationState, event.type, false, event.args);

                if (it->onTransition)
                {
                    it->onTransition(event.args);
                }

                res = HsmEventStatus_t::DONE_OK;
            }
        }

        // execute exit transition (only once in case of parallel transitions)
        for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it)
        {
            if (it->fromState != it->destinationState)
            {
                // it's a direct transition from currently active state
                if (fromState == it->fromState)
                {
                    isExitAllowed = onStateExiting(fromState);

                    if (true == isExitAllowed)
                    {
                        exitedStates.push_back(fromState);
                        mActiveStates.remove(fromState);
                    }
                    break;
                }
                // it's an outer transition from parent state. we need to find and exit all active substates
                else
                {
                    for (auto itActiveState = mActiveStates.begin(); itActiveState != mActiveStates.end(); ++itActiveState)
                    {
                        if (true == isSubstateOf(it->fromState, *itActiveState))
                        {
                            isExitAllowed = onStateExiting(*itActiveState);

                            if (true == isExitAllowed)
                            {
                                exitedStates.push_back(*itActiveState);
                            }
                            else
                            {
                                break;
                            }
                        }
                    }

                    // if no one blocked ongoing transition - remove child states from active list
                    if (true == isExitAllowed)
                    {
                        // store history for states between "fromState" ----> "it->fromState"
                        updateHistory(it->fromState, exitedStates);

                        for (auto itState = exitedStates.begin(); itState != exitedStates.end(); ++itState)
                        {
                            mActiveStates.remove(*itState);
                        }
                    }
                    // if one of the states blocked ongoing transition we need to rollback
                    else
                    {
                        for (auto itState = exitedStates.begin(); itState != exitedStates.end(); ++itState)
                        {
                            mActiveStates.remove(*itState);
                            // to prevent infinite loops we don't allow state to cancel transition
                            onStateEntering(*itState, VariantList_t());
                            mActiveStates.push_back(*itState);
                            onStateChanged(*itState, VariantList_t());
                        }
                    }
                }
            }
        }

        if (true == isExitAllowed)
        {
            for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it)
            {
                if (it->fromState != it->destinationState)
                {
                    // NOTE: Decide if we need functionality to cancel ongoing transition
                    logHsmAction((TransitionType::ENTRYPOINT != event.transitionType ? HsmLogAction::TRANSITION : HsmLogAction::TRANSITION_ENTRYPOINT),
                                 it->fromState, it->destinationState, event.type, false, event.args);

                    if (it->onTransition)
                    {
                        it->onTransition(event.args);
                    }

                    if (true == onStateEntering(it->destinationState, event.args))
                    {
                        std::list<HsmStateEnum> entryPoints;

                        if (true == replaceActiveState(fromState, it->destinationState))
                        {
                            onStateChanged(it->destinationState, event.args);
                        }

                        // we transitioned into history state
                        auto itHistoryData = mHistoryData.find(it->destinationState);

                        if (itHistoryData != mHistoryData.end())
                        {
                            __HSM_TRACE_DEBUG__("state=%d is a history state with %ld stored states",
                                            SC2INT(it->destinationState), itHistoryData->second.previousActiveStates.size());

                            // we need to transition to previous states or to default state
                            if (itHistoryData->second.previousActiveStates.size() > 0)
                            {
                                _HSM_SYNC_EVENTS_QUEUE();

                                PendingEventInfo historyTransitionEvent = event;

                                historyTransitionEvent.transitionType = TransitionType::FORCED;
                                historyTransitionEvent.forcedTransitionsInfo = std::make_shared<std::list<TransitionInfo>>();

                                for (auto itPrevState = itHistoryData->second.previousActiveStates.begin();
                                     itPrevState != itHistoryData->second.previousActiveStates.end();
                                     ++itPrevState)
                                {
                                    historyTransitionEvent.forcedTransitionsInfo->emplace_back(it->destinationState,
                                                                                               *itPrevState,
                                                                                               itHistoryData->second.transitionCallback,
                                                                                               nullptr);
                                }

                                itHistoryData->second.previousActiveStates.clear();
                                mPendingEvents.push_front(historyTransitionEvent);
                            }
                            else
                            {
                                std::list<HsmStateEnum> historyTargets;

                                if (INVALID_HSM_STATE_ID == itHistoryData->second.defaultTarget)
                                {
                                    HsmStateEnum historyParent;

                                    if (true == getHistoryParent(it->destinationState, historyParent))
                                    {
                                        __HSM_TRACE_DEBUG__("found parent=%d for history state=%d",
                                                        SC2INT(historyParent), SC2INT(it->destinationState));
                                        getEntryPoints(historyParent, event.type, historyTargets);
                                    }
                                    else
                                    {
                                        __HSM_TRACE_ERROR__("parent for history state=%d wasnt found", SC2INT(it->destinationState));
                                    }
                                }
                                else
                                {
                                    historyTargets.push_back(itHistoryData->second.defaultTarget);
                                }

                                PendingEventInfo defHistoryTransitionEvent = event;

                                defHistoryTransitionEvent.transitionType = TransitionType::FORCED;

                                for (HsmStateEnum defaultTargetState: historyTargets)
                                {
                                    __HSM_TRACE_LINE__();
                                    defHistoryTransitionEvent.forcedTransitionsInfo = std::make_shared<std::list<TransitionInfo>>();
                                    defHistoryTransitionEvent.forcedTransitionsInfo->emplace_back(it->destinationState,
                                                                                                  defaultTargetState,
                                                                                                  itHistoryData->second.transitionCallback,
                                                                                                  nullptr);
                                    mPendingEvents.push_front(defHistoryTransitionEvent);
                                }
                            }

                            res = HsmEventStatus_t::PENDING;
                        }
                        // check if new state has substates and initiate entry transition
                        else if (true == getEntryPoints(it->destinationState, event.type, entryPoints))
                        {
                            __HSM_TRACE_DEBUG__("state <%d> has substates with %d entry points (first: %d)",
                                            SC2INT(it->destinationState), SC2INT(entryPoints.size()), SC2INT(entryPoints.front()));
                            PendingEventInfo entryPointTransitionEvent = event;

                            entryPointTransitionEvent.transitionType = TransitionType::ENTRYPOINT;

                            {
                                _HSM_SYNC_EVENTS_QUEUE();
                                mPendingEvents.push_front(entryPointTransitionEvent);
                            }
                            res = HsmEventStatus_t::PENDING;
                        }
                        else
                        {
                            res = HsmEventStatus_t::DONE_OK;
                        }
                    }
                    else
                    {
                        for (auto itState = exitedStates.begin(); itState != exitedStates.end(); ++itState)
                        {
                            // to prevent infinite loops we don't allow state to cancel transition
                            onStateEntering(*itState, VariantList_t());
                            addActiveState(*itState);
                            onStateChanged(*itState, VariantList_t());
                        }
                    }
                }
            }
        }
        else
        {
            res = HsmEventStatus_t::CANCELED;
        }
    }

    if (HsmEventStatus_t::DONE_FAILED == res)
    {
        __HSM_TRACE_DEBUG__("event <%d> in state <%d> was ignored.", SC2INT(event.type), SC2INT(fromState));
    }

    __HSM_TRACE_CALL_RESULT__("%d", SC2INT(res));
    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::clearPendingEvents()
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("clearPendingEvents: mPendingEvents.size()=%ld", mPendingEvents.size());

    for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) ; ++it)
    {
        // since ongoing transitions can't be canceled we need to treat entry point transitions as atomic
        if (TransitionType::REGULAR == it->transitionType)
        {
            it->releaseLock();
        }
    }

    mPendingEvents.clear();
}

// ============================================================================
// PRIVATE: PendingEventInfo
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::~PendingEventInfo()
{
    if (true == cvLock.unique())
    {
        __HSM_TRACE_CALL_DEBUG_ARGS__("event=%d was deleted. releasing lock", SC2INT(type));
        unlock(HsmEventStatus_t::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::initLock()
{
    if (!cvLock)
    {
        cvLock = std::make_shared<std::mutex>();
        syncProcessed = std::make_shared<std::condition_variable>();
        transitionStatus = std::make_shared<HsmEventStatus_t>();
        *transitionStatus = HsmEventStatus_t::PENDING;
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::releaseLock()
{
    if (isSync())
    {
        __HSM_TRACE_CALL_DEBUG_ARGS__("releaseLock");
        unlock(HsmEventStatus_t::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::isSync()
{
    return (nullptr != cvLock);
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::wait(const int timeoutMs)
{
    if (isSync())
    {
        std::unique_lock<std::mutex> lck(*cvLock);

        __HSM_TRACE_CALL_DEBUG_ARGS__("trying to wait... (current status=%d, %p)", SC2INT(*transitionStatus), transitionStatus.get());
        if (timeoutMs > 0)
        {
            syncProcessed->wait_for(lck, std::chrono::milliseconds(timeoutMs),
                                    [=](){return (HsmEventStatus_t::PENDING != *transitionStatus);});
        }
        else
        {
            syncProcessed->wait(lck, [=](){return (HsmEventStatus_t::PENDING != *transitionStatus);});
        }

        __HSM_TRACE_DEBUG__("unlocked! transitionStatus=%d", SC2INT(*transitionStatus));
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::unlock(const HsmEventStatus_t status)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("try to unlock with status=%d", SC2INT(status));

    if (isSync())
    {
        __HSM_TRACE_DEBUG__("SYNC object (%p)", transitionStatus.get());
        *transitionStatus = status;

        if (status != HsmEventStatus_t::PENDING)
        {
            syncProcessed->notify_one();
        }
    }
    else
    {
        __HSM_TRACE_DEBUG__("ASYNC object");
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::hasSubstates(const HsmStateEnum parent) const
{
    return (mSubstates.find(parent) != mSubstates.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::hasEntryPoint(const HsmStateEnum state) const
{
    return (mSubstateEntryPoints.find(state) != mSubstateEntryPoints.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getEntryPoints(const HsmStateEnum state,
                                                                          const HsmEventEnum onEvent,
                                                                          std::list<HsmStateEnum>& outEntryPoints) const
{
    bool hasEntryPoint = false;
    auto itRange = mSubstateEntryPoints.equal_range(state);
    std::list<HsmStateEnum> nonconditionalEntryPoints;

    outEntryPoints.clear();

    for (auto it = itRange.first; it != itRange.second; ++it)
    {
        if (INVALID_HSM_EVENT_ID == it->second.onEvent)
        {
            nonconditionalEntryPoints.push_back(it->second.state);
        }
        else if (onEvent == it->second.onEvent)
        {
            outEntryPoints.push_back(it->second.state);
        }
    }

    if (outEntryPoints.empty() == true)
    {
        if (nonconditionalEntryPoints.empty() == false)
        {
            outEntryPoints = nonconditionalEntryPoints;
            hasEntryPoint = true;
        }
    }
    else
    {
        hasEntryPoint = true;
    }

    return hasEntryPoint;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::replaceActiveState(const HsmStateEnum oldState,
                                                                              const HsmStateEnum newState)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("oldState=%d, newState=%d", SC2INT(oldState), SC2INT(newState));

    mActiveStates.remove(oldState);

    return addActiveState(newState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::addActiveState(const HsmStateEnum newState)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("newState=%d", SC2INT(newState));
    bool wasAdded = false;

    if (false == isStateActive(newState))
    {
        mActiveStates.push_back(newState);
        wasAdded = true;
    }

    __HSM_TRACE_DEBUG__("mActiveStates.size=%d", SC2INT(mActiveStates.size()));
    return wasAdded;
}

#ifdef HSM_ENABLE_SAFE_STRUCTURE

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTopState(const HsmStateEnum state) const
{
    auto it = std::find(mTopLevelStates.begin(), mTopLevelStates.end(), state);

    return (it == mTopLevelStates.end());
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isSubstate(const HsmStateEnum state) const
{
    bool result = false;

    for (auto itSubstate = mSubstates.begin(); itSubstate != mSubstates.end(); ++itSubstate)
    {
        if (itSubstate->second == state)
        {
            result = true;
            break;
        }
    }

    return result;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::hasParentState(const HsmStateEnum state,
                                                                                           HsmStateEnum &outParent) const
{
    bool hasParent = false;

    for (auto it = mSubstates.begin(); it != mSubstates.end(); ++it)
    {
        if (state == it->second)
        {
            hasParent = true;
            outParent = it->first;
            break;
        }
    }

    return hasParent;
}

#endif // HSM_ENABLE_SAFE_STRUCTURE

template <typename HsmStateEnum, typename HsmEventEnum>
std::string HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getStateName(const HsmStateEnum state)
{
    std::string name;

    if (state != INVALID_HSM_STATE_ID)
    {
        name = std::to_string(static_cast<int>(state));
    }

    return name;
}

template <typename HsmStateEnum, typename HsmEventEnum>
std::string HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::getEventName(const HsmEventEnum event)
{
    std::string name;

    if (event != INVALID_HSM_EVENT_ID)
    {
        name = std::to_string(static_cast<int>(event));
    }

    return name;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::enableHsmDebugging()
{
#ifdef HSMBUILD_DEBUGGING
    char* envPath = std::getenv(ENV_DUMPPATH);

    return enableHsmDebugging(nullptr == envPath ? DEFAULT_DUMP_PATH : std::string(envPath));
#else
    return true;
#endif
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::enableHsmDebugging(const std::string& dumpPath)
{
#ifdef HSMBUILD_DEBUGGING
    bool res = false;
    bool isNewLog = (access(dumpPath.c_str(), F_OK) != 0);

    if (nullptr != mHsmLogFile.open(dumpPath.c_str(), std::ios::out | std::ios::app))
    {
        mHsmLog = std::make_shared<std::ostream>(&mHsmLogFile);

        if (true == isNewLog)
        {
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
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::disableHsmDebugging()
{
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
                                                                        const VariantList_t& args)
{
#ifdef HSMBUILD_DEBUGGING
    if (true == mHsmLogFile.is_open())
    {
        static const std::map<HsmLogAction, std::string> actionsMap = {std::make_pair(HsmLogAction::IDLE, "idle"),
                                                                       std::make_pair(HsmLogAction::TRANSITION, "transition"),
                                                                       std::make_pair(HsmLogAction::TRANSITION_ENTRYPOINT, "transition_entrypoint"),
                                                                       std::make_pair(HsmLogAction::CALLBACK_EXIT, "callback_exit"),
                                                                       std::make_pair(HsmLogAction::CALLBACK_ENTER, "callback_enter"),
                                                                       std::make_pair(HsmLogAction::CALLBACK_STATE, "callback_state")};
        char bufTime[80] = { 0 };
        char bufTimeMs[6] = { 0 };
        auto currentTimePoint = std::chrono::system_clock::now();
        const std::time_t tt = std::chrono::system_clock::to_time_t(currentTimePoint);
        std::tm timeinfo;
        const std::tm* tmResult = nullptr;// this is just to check that localtime was executed correctly

#ifdef WIN32
        if (0 == ::localtime_s(&timeinfo, &tt))
        {
            tmResult = &timeinfo;
        }
#else
        tmResult = localtime(&tt);
        if (nullptr != tmResult)
        {
            timeinfo = *tmResult;
        }
#endif // WIN32

        if (nullptr != tmResult)
        {
            std::strftime(bufTime, sizeof(bufTime), "%Y-%m-%d %H:%M:%S", &timeinfo);
            snprintf(bufTimeMs, sizeof(bufTimeMs), ".%03d",
                     static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint.time_since_epoch()).count() % 1000));
        }
        else
        {
            std::strcpy(bufTime, "0000-00-00 00:00:00");
            std::strcpy(bufTimeMs, ".000");
        }

        *mHsmLog << "\n-\n"
                    "  timestamp: \"" << bufTime << bufTimeMs << "\"\n"
                    "  active_states:";

        for (auto itState = mActiveStates.begin(); itState != mActiveStates.end(); ++itState)
        {
            *mHsmLog << "\n    - " << getStateName(*itState);
        }

        *mHsmLog << "\n  action: " << actionsMap.at(action) << "\n"
                    "  from_state: " << getStateName(fromState) << "\n"
                    "  target_state: " << getStateName(targetState) << "\n"
                    "  event: " << getEventName(event) << "\n"
                    "  status: " << (hasFailed ? "failed" : "") << "\n"
                    "  args:";

        for (auto itArg = args.begin() ; itArg != args.end(); ++itArg)
        {
            *mHsmLog << "\n    - " << itArg->toString();
        }

        mHsmLog->flush();
    }
#endif // HSMBUILD_DEBUGGING
}

} // namespace hsmcpp

#endif  // __HSMCPP_HSM_HPP__
