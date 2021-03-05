// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#ifndef __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <map>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <condition_variable>

class HsmEventDispatcherSTD: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherSTD();
    virtual ~HsmEventDispatcherSTD();

    virtual int registerEventHandler(std::function<void(void)> handler) override;
    virtual void unregisterEventHandler(const int handlerId) override;
    virtual void emit() override;

    virtual bool start() override;

protected:
    virtual void unregisterAllEventHandlers() override;
    void doDispatching();

private:
    std::thread mDispatcherThread;
    std::mutex mEmitSync;
    std::condition_variable mEmitEvent;
    std::mutex mStartupSync;
    std::condition_variable mStartupEvent;
    std::atomic<int> mPendingEmitCount;
    std::atomic<bool> mStopDispatcher;
    std::map<int, std::function<void(void)>> mEventHandlers;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERSTD_HPP__
