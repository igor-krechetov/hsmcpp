// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#include "HsmEventDispatcherGLibmm.hpp"

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm()
    : mDispatcher(std::make_shared<Glib::Dispatcher>())
{
}

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context)
    : mDispatcher(std::make_shared<Glib::Dispatcher>(context))
{
}

HsmEventDispatcherGLibmm::~HsmEventDispatcherGLibmm()
{
    unregisterAllEventHandlers();
}

int HsmEventDispatcherGLibmm::registerEventHandler(std::function<void(void)> handler)
{
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
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        it->second.disconnect();
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherGLibmm::emit()
{
    if (mDispatcher)
    {
        mDispatcher->emit();
    }
}

bool HsmEventDispatcherGLibmm::start()
{
    // NOTE: in case of GLib based dispatcher implementation it's expected
    //       that application will run GLib MainLoop in it's own code.
    //       start() function is added only for compatibility with other implementations.
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