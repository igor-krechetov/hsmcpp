// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <map>
#include <mutex>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>

class HsmEventDispatcherSTD: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherSTD();
    virtual ~HsmEventDispatcherSTD();

    virtual int registerEventHandler(std::function<void(void)> handler) override;
    virtual void unregisterEventHandler(const int handlerId) override;
    virtual void emitEvent() override;

    virtual bool start() override;
    void stop();

    // Blocks current thread until dispatcher is stopped.
    // NOTE: Make sure you call stop() before destroying dispatcher object if you use join() API.
    //       Failing to do so will result in an undefined behaviour.
    void join();

protected:
    void unregisterAllEventHandlers();
    void doDispatching();

private:
    std::thread mDispatcherThread;
    std::mutex mEmitSync;
    std::condition_variable mEmitEvent;
    std::mutex mStartupSync;
    std::condition_variable mStartupEvent;
    std::mutex mHandlersSync;
    std::map<int, std::function<void(void)>> mEventHandlers;
    int mPendingEmitCount = 0;
    bool mStopDispatcher = false;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__
