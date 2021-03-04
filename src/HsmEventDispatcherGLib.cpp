// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#include "HsmEventDispatcherGLib.hpp"

HsmEventDispatcherGLib::HsmEventDispatcherGLib()
    : mDispatcher(std::make_shared<Glib::Dispatcher>())
{
}

HsmEventDispatcherGLib::HsmEventDispatcherGLib(const Glib::RefPtr<Glib::MainContext>& context)
    : mDispatcher(std::make_shared<Glib::Dispatcher>(context))
{
}

HsmEventDispatcherGLib::~HsmEventDispatcherGLib()
{
    unregisterAllEventHandlers();
}

int HsmEventDispatcherGLib::registerEventHandler(std::function<void(void)> handler)
{
    int id = INVALID_HSM_DISPATCHER_HANDLER_ID;

    if (mDispatcher)
    {
        id = getNextHandlerID();
        mEventHandlers.emplace(id, mDispatcher->connect(handler));
    }

    return id;
}

void HsmEventDispatcherGLib::unregisterEventHandler(const int handlerId)
{
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        it->second.disconnect();
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherGLib::emit()
{
    if (mDispatcher)
    {
        mDispatcher->emit();
    }
}

bool HsmEventDispatcherGLib::start()
{
    // NOTE: in case of GLib based dispatcher implementation it's expected
    //       that application will run GLib MainLoop in it's own code.
    //       start() function is added only for compatibility with other implementations.
    return true;
}

void HsmEventDispatcherGLib::unregisterAllEventHandlers()
{
    for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
    {
        it->second.disconnect();
    }

    mEventHandlers.clear();
}