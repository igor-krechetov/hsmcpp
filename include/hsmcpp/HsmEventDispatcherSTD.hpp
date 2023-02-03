// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERSTD_HPP
#define HSMCPP_HSMEVENTDISPATCHERSTD_HPP

#include "HsmEventDispatcherBase.hpp"
#include <thread>
#include <map>
#include <chrono>
#include "os/ConditionVariable.hpp"

namespace hsmcpp
{

class HsmEventDispatcherSTD: public HsmEventDispatcherBase
{
private:
    struct RunningTimerInfo
    {
        std::chrono::time_point<std::chrono::steady_clock> startedAt;// monotonic time when timer was started (ms)
        std::chrono::time_point<std::chrono::steady_clock> elapseAfter;// monotonic time when timer should elapse next time (ms)
    };

public:
    // NOTE: false positive. setting default parameter value is not parameter modification
    // cppcheck-suppress misra-c2012-17.8
    explicit HsmEventDispatcherSTD(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);
    virtual ~HsmEventDispatcherSTD();

    virtual void emitEvent(const HandlerID_t handlerID) override;

    virtual bool start() override;
    void stop();

    // Blocks current thread until dispatcher is stopped.
    // NOTE: Make sure you call stop() before destroying dispatcher object if you use join() API.
    //       Failing to do so will result in an undefined behaviour.
    void join();

protected:
    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;


    void notifyDispatcherAboutEvent() override;
    void doDispatching();

    void notifyTimersThread();
    void handleTimers();

private:
    std::thread mDispatcherThread;
    std::thread mTimersThread;
    // NOTE: ideally it would be better to use a semaphore here, but there are no semaphores in C++11
    ConditionVariable mEmitEvent;
    ConditionVariable mTimerEvent;
    bool mStopDispatcher = false;
    bool mNotifiedTimersThread = false;
    std::map<TimerID_t, RunningTimerInfo> mRunningTimers;
    Mutex mRunningTimersSync;
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERSTD_HPP
