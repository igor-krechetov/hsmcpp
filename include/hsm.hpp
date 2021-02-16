// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#ifndef __HSMCPP_HSM_HPP__
#define __HSMCPP_HSM_HPP__

#include <glibmm.h>
#include <atomic>
#include <map>
#include <mutex>
#include <condition_variable>
#include "variant.hpp"
#include "logging.hpp"

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "HierarchicalStateMachine"

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
class HierarchicalStateMachine
{
public:
    typedef std::vector<Variant> VariantList_t;
    typedef void (HsmHandlerClass::*HsmTransitionCallback_t)(const VariantList_t&);
    typedef void (HsmHandlerClass::*HsmStateChangedCallback_t)(const VariantList_t&);
    typedef bool (HsmHandlerClass::*HsmStateEnterCallback_t)(const VariantList_t&);
    typedef bool (HsmHandlerClass::*HsmStateExitCallback_t)();

private:
    enum class EventStatus
    {
        PENDING,
        DONE_OK,
        DONE_FAILED
    };

    struct StateCallbacks
    {
        HsmHandlerClass* handler;
        HsmStateChangedCallback_t onStateChanged;
        HsmStateEnterCallback_t onEntering;
        HsmStateExitCallback_t onExiting;
    };

    struct TransitionInfo
    {
        HsmStateEnum destinationState;
        HsmHandlerClass* handler = nullptr;
        HsmTransitionCallback_t onTransition = nullptr;
    };

    struct PendingEventInfo
    {
        HsmEventEnum type;
        VariantList_t args;
        std::shared_ptr<std::mutex> cvLock;
        std::shared_ptr<std::condition_variable> syncProcessed;
        std::shared_ptr<EventStatus> transitionStatus;

        ~PendingEventInfo();
        void initLock();
        void releaseLock();
        bool isSync();
        void wait();
        void unlock(const EventStatus status);
    };

public:
    HierarchicalStateMachine(const HsmStateEnum initialState);
    virtual ~HierarchicalStateMachine();

    void registerState(const HsmStateEnum state,
                       HsmHandlerClass* handler,
                       HsmStateChangedCallback_t onStateChanged,
                       HsmStateEnterCallback_t onEntering,
                       HsmStateExitCallback_t onExiting);

    void registerSubstate(const HsmStateEnum substate, const HsmStateEnum childState);

    void registerTransition(const HsmStateEnum from,
                            const HsmStateEnum to,
                            const HsmEventEnum onEvent,
                            HsmHandlerClass* handler = nullptr,
                            HsmTransitionCallback_t transitionCallback = nullptr);

    HsmStateEnum getCurrentState() const;

    // extended version of transition function with all possible arguments
    template <typename... Args>
    bool transitionEx(const HsmEventEnum event, const bool clearQueue, const bool sync, Args... args);

    // regular async transition
    template <typename... Args>
    void transition(const HsmEventEnum event, Args... args);

    // async transition which clears events queue before adding requested event
    template <typename... Args>
    void transitionWithQueueClear(const HsmEventEnum event, Args... args);

    bool isTransitionPossible(const HsmEventEnum event);

private:
    template <typename... Args>
    void makeVariantList(VariantList_t& vList, Args&&... args);

    void dispatchEvents();

    bool onStateExiting(const HsmStateEnum state);
    bool onStateEntering(const HsmStateEnum state, const VariantList_t& args);
    void onStateChanged(const HsmStateEnum state, const VariantList_t& args);
    bool getParentState(const HsmStateEnum child, HsmStateEnum& outParent);
    bool findTransitionTarget(const HsmEventEnum event, TransitionInfo& outTransitionInfo);
    bool findTransitionTarget(HsmStateEnum curState, const HsmEventEnum event, TransitionInfo& outTransitionInfo);
    bool doTransition(const PendingEventInfo& event);
    void clearPendingEvents();

private:
    HsmStateEnum mCurrentState;
    std::map<std::pair<HsmStateEnum, HsmEventEnum>, TransitionInfo> mTransitionsByEvent; // FROM_STATE, EVENT => TO
    std::map<HsmStateEnum, StateCallbacks> mCallbacks;
    std::multimap<HsmStateEnum, HsmStateEnum> mSubstates;
    std::multimap<HsmStateEnum, HsmStateEnum> mSubstateEntryPoint;
    std::list<PendingEventInfo> mPendingEvents;
    Glib::Dispatcher mSignalTransition;
};

// ============================================================================
// PUBLIC
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::HierarchicalStateMachine(const HsmStateEnum initialState)
    : mCurrentState(initialState)
{
    mSignalTransition.connect(sigc::mem_fun(this, &HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::dispatchEvents));
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::~HierarchicalStateMachine()
{}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::registerState(const HsmStateEnum state,
                                                                                          HsmHandlerClass* handler,
                                                                                          HsmStateChangedCallback_t onStateChanged,
                                                                                          HsmStateEnterCallback_t onEntering,
                                                                                          HsmStateExitCallback_t onExiting)
{
    if ((nullptr != handler) &&
        ((nullptr != onStateChanged) || (nullptr != onEntering) || (nullptr != onExiting)))
    {
        StateCallbacks cb;

        cb.handler = handler;
        cb.onStateChanged = onStateChanged;
        cb.onEntering = onEntering;
        cb.onExiting = onExiting;
        mCallbacks[state] = cb;

        __TRACE_CALL_DEBUG_ARGS__("mCallbacks.size=%ld", mCallbacks.size());
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::registerSubstate(const HsmStateEnum substate,
                                                                                             const HsmStateEnum childState)
{
    mSubstates.insert(std::make_pair(substate, childState));
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::registerTransition(const HsmStateEnum from,
                                                                                               const HsmStateEnum to,
                                                                                               const HsmEventEnum onEvent,
                                                                                               HsmHandlerClass* handler,
                                                                                               HsmTransitionCallback_t transitionCallback)
{
    mTransitionsByEvent.emplace(std::make_pair(from, onEvent), TransitionInfo{to, handler, transitionCallback});
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
inline HsmStateEnum HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::getCurrentState() const
{
    return mCurrentState;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
template <typename... Args>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::transitionEx(const HsmEventEnum event,
                                                                                         const bool clearQueue,
                                                                                         const bool sync,
                                                                                         Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("transitionEx: event=%d", static_cast<int>(event));

    bool status = false;
    PendingEventInfo eventInfo;

    eventInfo.type = event;
    makeVariantList(eventInfo.args, args...);

    if (true == clearQueue)
    {
        clearPendingEvents();
    }

    if (true == sync)
    {
        eventInfo.initLock();
    }

    mPendingEvents.push_back(eventInfo);
    __TRACE_DEBUG__("transitionEx: emit");
    mSignalTransition.emit();

    if (true == sync)
    {
        __TRACE_DEBUG__("transitionEx: wait...");
        eventInfo.wait();
        status = (EventStatus::DONE_OK == *eventInfo.transitionStatus);
    }

    return status;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::transition(const HsmEventEnum event, Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("event with args = %d", static_cast<int>(event));

    transitionEx(event, false, false, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::transitionWithQueueClear(const HsmEventEnum event,
                                                                                                     Args... args)
{
    __TRACE_CALL_DEBUG_ARGS__("event=%d", static_cast<HsmEventEnum>(event));

    transitionEx(event, true, false, args...);
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::isTransitionPossible(const HsmEventEnum event)
{
    __TRACE_CALL_DEBUG_ARGS__("event = %d", static_cast<HsmEventEnum>(event));

    HsmStateEnum stateFrom = mCurrentState;
    TransitionInfo transitionDestination;
    HsmEventEnum nextEvent;
    bool possible = true;

    for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) && (true == possible); ++it)
    {
        nextEvent = it->type;
        possible = findTransitionTarget(stateFrom, nextEvent, transitionDestination);

        if (true == possible)
        {
            stateFrom = transitionDestination.destinationState;
        }
    }

    if (true == possible)
    {
        nextEvent = event;
        possible = findTransitionTarget(stateFrom, nextEvent, transitionDestination);
    }

    __TRACE_CALL_RESULT__("%d", BOOL2INT(possible));
    return possible;
}

// ============================================================================
// PRIVATE
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
template <typename... Args>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::makeVariantList(VariantList_t& vList, Args&&... args)
{
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::dispatchEvents()
{
    __TRACE_CALL_DEBUG_ARGS__("dispatchEvents: mPendingEvents.size() = %ld", mPendingEvents.size());

    if (false == mPendingEvents.empty())
    {
        PendingEventInfo pendingEvent = mPendingEvents.front();
        bool transitiontStatus;

        mPendingEvents.pop_front();
        transitiontStatus = doTransition(pendingEvent);

        if (true == pendingEvent.isSync())
        {
            __TRACE_DEBUG__("dispatchEvents: sync transition. unlock with status %d", BOOL2INT(transitiontStatus));
            pendingEvent.unlock(true == transitiontStatus ? EventStatus::DONE_OK : EventStatus::DONE_FAILED);
        }
    }

    if (false == mPendingEvents.empty())
    {
        mSignalTransition.emit();
    }
    else
    {
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::onStateExiting(const HsmStateEnum state)
{
    bool res = true;
    auto it = mCallbacks.find(state);

    if ((mCallbacks.end() != it) && (nullptr != it->second.onExiting))
    {
        res = ((it->second.handler)->*(it->second.onExiting))();
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::onStateEntering(const HsmStateEnum state,
                                                                                            const VariantList_t& args)
{
    bool res = true;
    auto it = mCallbacks.find(state);

    if ((mCallbacks.end() != it) && (nullptr != it->second.onEntering))
    {
        res = ((it->second.handler)->*(it->second.onEntering))(args);
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::onStateChanged(const HsmStateEnum state,
                                                                                           const VariantList_t& args)
{
    __TRACE_CALL_DEBUG_ARGS__("onStateChanged with state <%d>", static_cast<int>(state));
    auto it = mCallbacks.find(state);

    if ((mCallbacks.end() != it) && (nullptr != it->second.onStateChanged))
    {
        ((it->second.handler)->*(it->second.onStateChanged))(args);
    }
    else
    {
        __TRACE_DEBUG__("WARNING: no callback registered for state <%d>", static_cast<int>(state));
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::getParentState(const HsmStateEnum child,
                                                                                           HsmStateEnum& outParent)
{
    bool wasFound = false;

    for (auto it : mSubstates)
    {
        if (child == it.second)
        {
            outParent = it.first;
            wasFound = true;
            break;
        }
    }

    return wasFound;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::findTransitionTarget(const HsmEventEnum event,
                                                                                                 TransitionInfo& outTransitionInfo)
{
    return findTransitionTarget(mCurrentState, event, outTransitionInfo);
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::findTransitionTarget(HsmStateEnum curState,
                                                                                                 const HsmEventEnum event,
                                                                                                 TransitionInfo& outTransitionInfo)
{
    bool wasFound = false;
    bool continueSearch;

    do
    {
        auto key = std::make_pair(curState, event);
        auto it = mTransitionsByEvent.find(key);

        continueSearch = false;

        if (mTransitionsByEvent.end() == it)
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
            outTransitionInfo = it->second;
            wasFound = true;
        }
    } while (true == continueSearch);

    return wasFound;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::doTransition(const PendingEventInfo& event)
{
    bool res = false;
    TransitionInfo transitionInfo;

    if (true == findTransitionTarget(event.type, transitionInfo))
    {
        if (mCurrentState != transitionInfo.destinationState)
        {
            // NOTE: Decide if we need functionality to cancel ongoing transition
            if (true == onStateExiting(mCurrentState))
            {
                if ((nullptr != transitionInfo.handler) && (transitionInfo.onTransition))
                {
                    ((transitionInfo.handler)->*(transitionInfo.onTransition))(event.args);
                }

                if (true == onStateEntering(transitionInfo.destinationState, event.args))
                {
                    mCurrentState = transitionInfo.destinationState;
                    onStateChanged(transitionInfo.destinationState, event.args);
                    res = true;
                }
                else
                {
                    // to prevent infinite loops we don't allow state to cancel transition
                    onStateEntering(mCurrentState, VariantList_t());
                    onStateChanged(mCurrentState, VariantList_t());
                }
            }
        }
        else if (nullptr != transitionInfo.onTransition)
        {
            ((transitionInfo.handler)->*(transitionInfo.onTransition))(event.args);
            res = true;
        }
    }

    if (false == res)
    {
        __TRACE_CALL_DEBUG_ARGS__("event <%d> in state <%d> was ignored.", static_cast<int>(event.type), static_cast<int>(mCurrentState));
    }

    return res;
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::clearPendingEvents()
{
    __TRACE_CALL_DEBUG_ARGS__("clearPendingEvents: mPendingEvents.size() = %ld", mPendingEvents.size());

    for (auto it = mPendingEvents.begin(); (it != mPendingEvents.end()) ; ++it)
    {
        it->releaseLock();
    }

    mPendingEvents.clear();
}

// ============================================================================
// PRIVATE: PendingEventInfo
// ============================================================================
template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::PendingEventInfo::~PendingEventInfo()
{
    if (true == cvLock.unique())
    {
        __TRACE_CALL_DEBUG_ARGS__("event=%d was deleted. releasing lock", static_cast<int>(type));
        unlock(EventStatus::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::PendingEventInfo::initLock()
{
    if (!cvLock)
    {
        cvLock = std::make_shared<std::mutex>();
        syncProcessed = std::make_shared<std::condition_variable>();
        transitionStatus = std::make_shared<EventStatus>();
        *transitionStatus = EventStatus::PENDING;
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::PendingEventInfo::releaseLock()
{
    if (isSync())
    {
        __TRACE_CALL_DEBUG_ARGS__("releaseLock");
        unlock(EventStatus::DONE_FAILED);
        cvLock.reset();
        syncProcessed.reset();
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
bool HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::PendingEventInfo::isSync()
{
    return (nullptr != cvLock);
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::PendingEventInfo::wait()
{
    if (isSync())
    {
        std::unique_lock<std::mutex> lck(*cvLock);

        while (EventStatus::PENDING == *transitionStatus)
        {
            __TRACE_CALL_DEBUG_ARGS__("trying to wait...");
            syncProcessed->wait(lck);
            __TRACE_DEBUG__("unlocked! transitionStatus=%d", (int)*transitionStatus);
        }
    }
}

template <typename HsmStateEnum, typename HsmEventEnum, class HsmHandlerClass>
void HierarchicalStateMachine<HsmStateEnum, HsmEventEnum, HsmHandlerClass>::PendingEventInfo::unlock(const EventStatus status)
{
    __TRACE_CALL_DEBUG_ARGS__("try to unlock with status=%d", (int)status);

    if (isSync())
    {
        __TRACE_DEBUG__("SYNC object");
        *transitionStatus = status;
        syncProcessed->notify_one();
    }
    else
    {
        __TRACE_DEBUG__("A-SYNC object");
    }
}

#endif  // __HSMCPP_HSM_HPP__