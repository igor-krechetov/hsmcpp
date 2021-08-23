// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <thread>
#include <condition_variable>

namespace hsmcpp
{

class HsmEventDispatcherSTD: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherSTD();
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
    std::condition_variable mEmitEvent;
    std::mutex mStartupSync;
    std::condition_variable mStartupEvent;
    bool mStopDispatcher = false;
};

} // namespace hsmcpp

#endif // __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__
