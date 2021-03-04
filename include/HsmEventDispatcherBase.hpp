// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#ifndef __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__

#include "IHsmEventDispatcher.hpp"

class HsmEventDispatcherBase: public IHsmEventDispatcher
{
public:
    virtual ~HsmEventDispatcherBase();

    // Used to start event dispatching. Implementation is optional and depends
    // on individual dispatcher. But it should be non blocking.
    virtual bool start() = 0;

protected:
    virtual int getNextHandlerID();
    // For internal usage. Must be called in destructor by all dispatchers.
    virtual void unregisterAllEventHandlers() = 0;

private:
    int mNextHandlerId = 1;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERBASE_HPP__
