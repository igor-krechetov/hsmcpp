// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_IHSMEVENTDISPATCHER_HPP__
#define __HSMCPP_IHSMEVENTDISPATCHER_HPP__

#include <functional>

namespace hsmcpp
{
#define INVALID_HSM_DISPATCHER_HANDLER_ID          (0)

class IHsmEventDispatcher
{
public:
    virtual ~IHsmEventDispatcher() {}

    // Used to start event dispatching. Implementation is optional and depends
    // on individual dispatcher. But it should be non blocking.
    // Returns TRUE if dispatching was successfully started or if it is already running (calling multiple times will have no effect)
    virtual bool start() = 0;

    // As a general rule registerEventHandler and unregisterEventHandler are
    // not expected to be thread-safe and should to be used from the same thread.
    // But this depends on specific Dispatcher implementation.

    // returns ID that should be used to unregister event handler
    virtual int registerEventHandler(std::function<void(void)> handler) = 0;

    virtual void unregisterEventHandler(const int handlerId) = 0;

    // dispatcher must guarantee that emit call is thread-safe
    virtual void emitEvent() = 0;
};

}// namespace hsmcpp

#endif // __HSMCPP_IHSMEVENTDISPATCHER_HPP__
