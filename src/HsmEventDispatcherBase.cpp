// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#include "HsmEventDispatcherBase.hpp"

HsmEventDispatcherBase::~HsmEventDispatcherBase()
{
}

int HsmEventDispatcherBase::getNextHandlerID()
{
    return mNextHandlerId++;
}