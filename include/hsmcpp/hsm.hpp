// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSM_HPP__
#define __HSMCPP_HSM_HPP__

#include <algorithm>
#include <functional>
#include <atomic>
#include <map>
#include <list>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "IHsmEventDispatcher.hpp"
#include "variant.hpp"
#include "logging.hpp"

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

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "HierarchicalStateMachine"

#define HSM_WAIT_INDEFINITELY                   (0)
#define INVALID_HSM_EVENT_ID                    static_cast<HsmEventEnum>(-1000)
#define INVALID_HSM_STATE_ID                    static_cast<HsmStateEnum>(-1000)

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

private:
    enum class HsmEventStatus
    {
        PENDING,
        DONE_OK,
        DONE_FAILED,
        CANCELED
    };

    // NOTE: just an alias to make code more readable
    #define HsmEventStatus_t   HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HsmEventStatus

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
        HsmStateEnum fromState;
        HsmStateEnum destinationState;
        HsmTransitionCallback_t onTransition = nullptr;
        HsmTransitionConditionCallback_t checkCondition = nullptr;
    };

    struct PendingEventInfo
    {
        bool entryPointTransition = false;
        HsmEventEnum type = static_cast<HsmEventEnum>(-1);
        VariantList_t args;
        std::shared_ptr<std::mutex> cvLock;
        std::shared_ptr<std::condition_variable> syncProcessed;
        std::shared_ptr<HsmEventStatus> transitionStatus;

        ~PendingEventInfo();
        void initLock();
        void releaseLock();
        bool isSync();
        void wait(const int timeoutMs = HSM_WAIT_INDEFINITELY);
        void unlock(const HsmEventStatus status);
    };

public:
    explicit HierarchicalStateMachine(const HsmStateEnum initialState);
    // Uses unregisterEventHandler from Dispatcher. Usually HSM has to be destroyed from the same thread it was created.
    virtual ~HierarchicalStateMachine();

    // Uses registerEventHandler from Dispatcher. Usually must be called from the same thread where dispatcher was created.
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

private:
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

private:
    HsmStateEnum mInitialState;
    std::list<HsmStateEnum> mActiveStates;
    std::multimap<std::pair<HsmStateEnum, HsmEventEnum>, TransitionInfo> mTransitionsByEvent; // FROM_STATE, EVENT => TO
    std::map<HsmStateEnum, StateCallbacks> mRegisteredStates;
    std::multimap<HsmStateEnum, HsmStateEnum> mSubstates;
    std::multimap<HsmStateEnum, StateEntryPoint> mSubstateEntryPoints;
    std::list<PendingEventInfo> mPendingEvents;
    std::shared_ptr<IHsmEventDispatcher> mDispatcher;
    int mDispatcherHandlerId = INVALID_HSM_DISPATCHER_HANDLER_ID;
    bool mStopDispatching = false;

#ifdef HSM_ENABLE_SAFE_STRUCTURE
    std::list<HsmStateEnum> mTopLevelStates; // list of states which are not substates and dont have substates of their own
#endif // HSM_ENABLE_SAFE_STRUCTURE

#ifndef HSM_DISABLE_THREADSAFETY
    std::mutex mEventsSync;
#endif// HSM_DISABLE_THREADSAFETY
};

// ============================================================================
// PUBLIC
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::HierarchicalStateMachine(const HsmStateEnum initialState)
    : mInitialState(initialState)
{
    __TRACE_INIT__();
    mActiveStates.push_back(mInitialState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::~HierarchicalStateMachine()
{
    release();
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::initialize(const std::shared_ptr<IHsmEventDispatcher>& dispatcher)
{
    __TRACE_CALL_DEBUG__();
    bool result = false;

    if (dispatcher)
    {
        if (true == dispatcher->start())
        {
            mDispatcher = dispatcher;
            mDispatcherHandlerId = mDispatcher->registerEventHandler(std::bind(&HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchEvents,
                                                                     this));

            result = (INVALID_HSM_DISPATCHER_HANDLER_ID != mDispatcherHandlerId);
        }
        else
        {
            __TRACE_ERROR__("failed to start dispatcher");
        }
    }
    else
    {
        __TRACE_ERROR__("dispatcher is NULL");
    }

    return result;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::release()
{
    mStopDispatching = true;
    __TRACE_CALL_DEBUG__();

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

        __TRACE_CALL_DEBUG_ARGS__("mRegisteredStates.size=%ld", mRegisteredStates.size());
    }
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
                    __TRACE_CALL_DEBUG_ARGS__("requested operation will result in substates recursion (parent=%d, substate=%d)",
                                              SC2INT(parent), SC2INT(substate));
                    registrationAllowed = false;
                    break;
                }

                curState = prevState;
            }
        }
        else
        {
            __TRACE_CALL_DEBUG_ARGS__("substate %d already has a parent %d",
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
    mTransitionsByEvent.emplace(std::make_pair(from, onEvent), TransitionInfo{from, to, transitionCallback, conditionCallback});
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
    __TRACE_CALL_DEBUG_ARGS__("transitionEx: event=%d, clearQueue=%s, sync=%s", SC2INT(event), BOOL2STR(clearQueue), BOOL2STR(sync));

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

    __TRACE_DEBUG__("transitionEx: emit");
    mDispatcher->emitEvent();

    if (true == sync)
    {
        __TRACE_DEBUG__("transitionEx: wait...");
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
    __TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));

    transitionEx(event, false, false, 0, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionSync(const HsmEventEnum event, const int timeoutMs, Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));
    return transitionEx(event, false, true, timeoutMs, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::transitionWithQueueClear(const HsmEventEnum event,
                                                                                    Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));

    transitionEx(event, true, false, 0, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTransitionPossible(const HsmEventEnum event,
                                                                                Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));
    bool possible = false;

    for (auto it = mActiveStates.begin() ; it != mActiveStates.end() ; ++it)
    {
        possible = isTransitionPossible(*it, event, args...);

        if (true == possible)
        {
            break;
        }
    }

    __TRACE_CALL_RESULT__("%d", BOOL2INT(possible));
    return possible;
}

// ============================================================================
// PRIVATE
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::makeVariantList(VariantList_t& vList, Args&&... args)
{
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::dispatchEvents()
{
    __TRACE_CALL_DEBUG_ARGS__("dispatchEvents: mPendingEvents.size()=%ld", mPendingEvents.size());

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

            __TRACE_DEBUG__("dispatchEvents: unlock with status %d", SC2INT(transitiontStatus));
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
        }
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::onStateChanged(const HsmStateEnum state,
                                                                          const VariantList_t& args)
{
    __TRACE_CALL_DEBUG_ARGS__("state=%d", SC2INT(state));
    auto it = mRegisteredStates.find(state);

    if ((mRegisteredStates.end() != it) && it->second.onStateChanged)
    {
        it->second.onStateChanged(args);
    }
    else
    {
        __TRACE_WARNING__("no callback registered for state <%d>", SC2INT(state));
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
    __TRACE_CALL_DEBUG_ARGS__("parent=%d, child=%d", SC2INT(parent), SC2INT(child));
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
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::isTransitionPossible(const HsmStateEnum fromState,
                                                                                const HsmEventEnum event,
                                                                                Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("event=%d", SC2INT(event));

    HsmStateEnum currentState = fromState;
    TransitionInfo possibleTransition;
    HsmEventEnum nextEvent;
    VariantList_t transitionArgs;
    bool possible = true;

    makeVariantList(transitionArgs, args...);

    {
        _HSM_SYNC_EVENTS_QUEUE();

        for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) && (true == possible); ++it)
        {
            nextEvent = it->type;
            possible = findTransitionTarget(currentState, nextEvent, transitionArgs, possibleTransition);

            if (true == possible)
            {
                currentState = possibleTransition.destinationState;
            }
        }
    }

    if (true == possible)
    {
        nextEvent = event;
        possible = findTransitionTarget(currentState, nextEvent, transitionArgs, possibleTransition);
    }

    __TRACE_CALL_RESULT__("%d", BOOL2INT(possible));
    return possible;
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::findTransitionTarget(const HsmStateEnum fromState,
                                                                                const HsmEventEnum event,
                                                                                const VariantList_t& transitionArgs,
                                                                                std::list<TransitionInfo>& outTransitions)
{
    __TRACE_CALL_DEBUG_ARGS__("fromState=%d, event=%d", SC2INT(fromState), SC2INT(event));
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
                __TRACE_DEBUG__("check transition to %d...", SC2INT(it->second.destinationState));

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
                                __TRACE_DEBUG__("state <%d> has entrypoints", SC2INT(currentParent));
                                std::list<HsmStateEnum> entryPoints;

                                if (true == getEntryPoints(currentParent, event, entryPoints))
                                {
                                    parentStates.splice(parentStates.end(), entryPoints);
                                }
                                else
                                {
                                    __TRACE_WARNING__("no matching entrypoints found");
                                    break;
                                }
                            }
                            else
                            {
                                __TRACE_WARNING__("state <%d> doesn't have an entrypoint defined", SC2INT(currentParent));
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

    __TRACE_CALL_RESULT__("%s", BOOL2STR(outTransitions.empty() == false));
    return (outTransitions.empty() == false);
}

template <typename HsmStateEnum, typename HsmEventEnum>
typename HsmEventStatus_t HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::doTransition(const PendingEventInfo& event)
{
    __TRACE_CALL_DEBUG_ARGS__("event=%d, entryPointTransition=%s", SC2INT(event.type), BOOL2STR(event.entryPointTransition));
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

    __TRACE_CALL_RESULT__("%d", SC2INT(res));
    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
typename HsmEventStatus_t HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::handleSingleTransition(const HsmStateEnum activeState,
                                                                                                       const PendingEventInfo& event)
{
    __TRACE_CALL_DEBUG_ARGS__("activeState=%d, event=%d, entryPointTransition=%s",
                              SC2INT(activeState), SC2INT(event.type), BOOL2STR(event.entryPointTransition));
    HsmEventStatus_t res = HsmEventStatus_t::DONE_FAILED;
    const HsmStateEnum fromState = activeState;
    bool isCorrectTransition = false;
    std::list<TransitionInfo> matchingTransitions;

    if (true == event.entryPointTransition)
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
            __TRACE_WARNING__("state <%d> doesn't have a suitable entry point (event <%d>)",
                              SC2INT(fromState), SC2INT(event.type));
        }
    }
    else
    {
        isCorrectTransition = findTransitionTarget(fromState, event.type, event.args, matchingTransitions);

        if (false == isCorrectTransition)
        {
            __TRACE_WARNING__("no suitable transition from state <%d> with event <%d>",
                              SC2INT(fromState), SC2INT(event.type));
        }
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
                it->onTransition(event.args);
                res = HsmEventStatus_t::DONE_OK;
            }
        }

        // execute exit transition (only once in case of parallel transitions)
        for (auto it = matchingTransitions.begin(); it != matchingTransitions.end(); ++it)
        {
            if (it->fromState != it->destinationState)
            {
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
                // we need to find and exit all active substates if it's an outer transition
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

                        if (true == getEntryPoints(it->destinationState, event.type, entryPoints))
                        {
                            __TRACE_DEBUG__("state <%d> has substates with %d entry points (first: %d)",
                                            SC2INT(it->destinationState), SC2INT(entryPoints.size()), SC2INT(entryPoints.front()));
                            PendingEventInfo entryPointTransitionEvent = event;

                            entryPointTransitionEvent.entryPointTransition = true;

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
        __TRACE_DEBUG__("event <%d> in state <%d> was ignored.", SC2INT(event.type), SC2INT(fromState));
    }

    __TRACE_CALL_RESULT__("%d", SC2INT(res));
    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::clearPendingEvents()
{
    __TRACE_CALL_DEBUG_ARGS__("clearPendingEvents: mPendingEvents.size()=%ld", mPendingEvents.size());

    for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) ; ++it)
    {
        // since ongoing transitions can't be canceled we need to treat entry point transitions as atomic
        if (false == it->entryPointTransition)
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
        __TRACE_CALL_DEBUG_ARGS__("event=%d was deleted. releasing lock", SC2INT(type));
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
        __TRACE_CALL_DEBUG_ARGS__("releaseLock");
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

        __TRACE_CALL_DEBUG_ARGS__("trying to wait... (current status=%d, %p)", SC2INT(*transitionStatus), transitionStatus.get());
        if (timeoutMs > 0)
        {
            syncProcessed->wait_for(lck, std::chrono::milliseconds(timeoutMs),
                                    [=](){return (HsmEventStatus_t::PENDING != *transitionStatus);});
        }
        else
        {
            syncProcessed->wait(lck, [=](){return (HsmEventStatus_t::PENDING != *transitionStatus);});
        }

        __TRACE_DEBUG__("unlocked! transitionStatus=%d", SC2INT(*transitionStatus));
    }
}

template <typename HsmStateEnum, typename HsmEventEnum>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::PendingEventInfo::unlock(const HsmEventStatus_t status)
{
    __TRACE_CALL_DEBUG_ARGS__("try to unlock with status=%d", SC2INT(status));

    if (isSync())
    {
        __TRACE_DEBUG__("SYNC object (%p)", transitionStatus.get());
        *transitionStatus = status;

        if (status != HsmEventStatus_t::PENDING)
        {
            syncProcessed->notify_one();
        }
    }
    else
    {
        __TRACE_DEBUG__("ASYNC object");
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
    __TRACE_CALL_DEBUG_ARGS__("oldState=%d, newState=%d", SC2INT(oldState), SC2INT(newState));

    mActiveStates.remove(oldState);

    return addActiveState(newState);
}

template <typename HsmStateEnum, typename HsmEventEnum>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum>::addActiveState(const HsmStateEnum newState)
{
    __TRACE_CALL_DEBUG_ARGS__("newState=%d", SC2INT(newState));
    bool wasAdded = false;

    if (false == isStateActive(newState))
    {
        mActiveStates.push_back(newState);
        wasAdded = true;
    }

    __TRACE_DEBUG__("mActiveStates.size=%d", SC2INT(mActiveStates.size()));
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

#endif  // __HSMCPP_HSM_HPP__
