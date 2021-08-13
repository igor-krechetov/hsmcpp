// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"
#include "hsmcpp/logging.hpp"

namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherGLibmm"

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm()
    : mMainContext(Glib::MainContext::get_default())
    , mDispatcher(std::make_unique<Glib::Dispatcher>())
{
    __HSM_TRACE_CALL_DEBUG__();

}

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context)
    : mMainContext(context)
    , mDispatcher(std::make_unique<Glib::Dispatcher>(context))
{
    __HSM_TRACE_CALL_DEBUG__();
}

HsmEventDispatcherGLibmm::~HsmEventDispatcherGLibmm()
{
    __HSM_TRACE_CALL_DEBUG__();
    
    unregisterAllTimerHandlers();
    unregisterAllEventHandlers();
}

HandlerID_t HsmEventDispatcherGLibmm::registerEventHandler(const EventHandlerFunc_t& handler)
{
    __HSM_TRACE_CALL_DEBUG__();
    HandlerID_t id = INVALID_HSM_DISPATCHER_HANDLER_ID;

    if (mDispatcher)
    {
        id = getNextHandlerID();
        mEventHandlers.emplace(id, mDispatcher->connect(handler));
    }

    return id;
}

void HsmEventDispatcherGLibmm::unregisterEventHandler(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("handlerId=%d", handlerID);
    auto it = mEventHandlers.find(handlerID);

    if (it != mEventHandlers.end())
    {
        it->second.disconnect();
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherGLibmm::emitEvent()
{
    __HSM_TRACE_CALL_DEBUG__();
    if (mDispatcher)
    {
        mDispatcher->emit();
    }
}

bool HsmEventDispatcherGLibmm::start()
{
    // do nothing
    // NOTE: in case of GLib based dispatcher implementation it's expected
    //       that application will run GLib MainLoop in it's own code.
    return true;
}

void HsmEventDispatcherGLibmm::unregisterAllEventHandlers()
{
    for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
    {
        it->second.disconnect();
    }

    mEventHandlers.clear();
}

void HsmEventDispatcherGLibmm::unregisterAllTimerHandlers()
{
    for (auto it = mTimerHandlers.begin(); it != mTimerHandlers.end(); ++it)
    {
        it->second.disconnect();
    }

    mTimerHandlers.clear();
}

void HsmEventDispatcherGLibmm::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d, intervalMs=%d, isSingleShot=%d",
                                  SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));
    auto it = mTimerHandlers.find(timerID);

    if (mTimerHandlers.end() == it)
    {
        sigc::connection newTimerConnection;

        newTimerConnection = mMainContext->signal_timeout().connect(sigc::bind(sigc::mem_fun(this, &HsmEventDispatcherGLibmm::onTimerEvent), timerID), intervalMs);
        mTimerHandlers.emplace(timerID, newTimerConnection);
    }
    else
    {
        __HSM_TRACE_ERROR__("timer with id=%d already exists", timerID);
    }
}

void HsmEventDispatcherGLibmm::stopTimerImpl(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    auto it = mTimerHandlers.find(timerID);

    if (mTimerHandlers.end() != it)
    {
        __HSM_TRACE_LINE__();
        it->second.disconnect();
        mTimerHandlers.erase(it);
    }
}

bool HsmEventDispatcherGLibmm::onTimerEvent(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    bool restartTimer = false;
    TimerInfo curTimer = getTimerInfo(timerID);

    __HSM_TRACE_DEBUG__("curTimer.handlerID=%d", curTimer.handlerID);

    if (INVALID_HSM_DISPATCHER_HANDLER_ID != curTimer.handlerID)
    {
        TimerHandlerFunc_t timerHandler = getTimerHandlerFunc(curTimer.handlerID);

        timerHandler(timerID);

        restartTimer = (true == curTimer.isSingleShot ? false : true);
    }

    if (false == restartTimer)
    {
        auto itTimer = mTimerHandlers.find(timerID);

        if (mTimerHandlers.end() != itTimer)
        {
            mTimerHandlers.erase(itTimer);
        }
        else
        {
            __HSM_TRACE_ERROR__("unexpected error. timer not found");
        }
    }

    return restartTimer;
}

} // namespace hsmcpp