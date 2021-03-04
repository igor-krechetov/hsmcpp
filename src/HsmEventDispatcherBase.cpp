// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#include "HsmEventDispatcherBase.hpp"

HsmEventDispatcherBase::~HsmEventDispatcherBase()
{
}

bool HsmEventDispatcherBase::start()
{
    // NOTE: in case of GLib based dispatcher implementation it's expected
    //       that application will run GLib MainLoop in it's own code.
    //       start() function is added only for compatibility with other implementations.
    return true;
}

int HsmEventDispatcherBase::getNextHandlerID()
{
    return mNextHandlerId++;
}