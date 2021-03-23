// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__

#include "IHsmEventDispatcher.hpp"

class HsmEventDispatcherBase: public IHsmEventDispatcher
{
public:
    virtual ~HsmEventDispatcherBase();

protected:
    virtual int getNextHandlerID();

private:
    int mNextHandlerId = 1;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__
