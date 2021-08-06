// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherBase.hpp"

namespace hsmcpp
{

HsmEventDispatcherBase::~HsmEventDispatcherBase()
{
}

int HsmEventDispatcherBase::getNextHandlerID()
{
    return mNextHandlerId++;
}

} // namespace hsmcpp