// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"
#include "hsmcpp/logging.hpp"

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "HsmEventDispatcherGLibmm"

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm()
    : mDispatcher(std::make_unique<Glib::Dispatcher>())
{
    __TRACE_CALL_DEBUG__();
}

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context)
    : mDispatcher(std::make_unique<Glib::Dispatcher>(context))
{
    __TRACE_CALL_DEBUG__();
}

HsmEventDispatcherGLibmm::~HsmEventDispatcherGLibmm()
{
    __TRACE_CALL_DEBUG__();
    unregisterAllEventHandlers();
}

int HsmEventDispatcherGLibmm::registerEventHandler(std::function<void(void)> handler)
{
    __TRACE_CALL_DEBUG__();
    int id = INVALID_HSM_DISPATCHER_HANDLER_ID;

    if (mDispatcher)
    {
        id = getNextHandlerID();
        mEventHandlers.emplace(id, mDispatcher->connect(handler));
    }

    return id;
}

void HsmEventDispatcherGLibmm::unregisterEventHandler(const int handlerId)
{
    __TRACE_CALL_DEBUG_ARGS__("handlerId=%d", handlerId);
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        it->second.disconnect();
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherGLibmm::emit()
{
    __TRACE_CALL_DEBUG__();
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

void HsmEventDispatcherGLibmm::stop()
{
    // do nothing
    // NOTE: in case of GLib based dispatcher implementation it's expected
    //       that application will run GLib MainLoop in it's own code.
}

void HsmEventDispatcherGLibmm::unregisterAllEventHandlers()
{
    for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
    {
        it->second.disconnect();
    }

    mEventHandlers.clear();
}