// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERSTD_HPP
#define HSMCPP_HSMEVENTDISPATCHERSTD_HPP

#include "HsmEventDispatcherBase.hpp"
#include <thread>
#include "os/ConditionVariable.hpp"

namespace hsmcpp
{

class HsmEventDispatcherSTD: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherSTD() = default;
    virtual ~HsmEventDispatcherSTD();

    virtual void emitEvent(const HandlerID_t handlerID) override;

    virtual bool start() override;
    void stop();

    // Blocks current thread until dispatcher is stopped.
    // NOTE: Make sure you call stop() before destroying dispatcher object if you use join() API.
    //       Failing to do so will result in an undefined behaviour.
    void join();

protected:
    void doDispatching();

private:
    std::thread mDispatcherThread;
    // NOTE: ideally it would be better to use a semaphore here, but there are no semaphores in C++11
    ConditionVariable mEmitEvent;
    bool mStopDispatcher = false;
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERSTD_HPP
