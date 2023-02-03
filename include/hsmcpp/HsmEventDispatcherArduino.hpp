// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP
#define HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP

#include "HsmEventDispatcherBase.hpp"
#include <vector>
#include <map>

namespace hsmcpp
{

class HsmEventDispatcherArduino: public HsmEventDispatcherBase
{
    struct RunningTimerInfo
    {
        unsigned long startedAt;// monotonic time when timer was started (ms)
        unsigned long elapseAfter;// monotonic time when timer should elapse next time (ms)
    };

public:
    explicit HsmEventDispatcherArduino(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);
    virtual ~HsmEventDispatcherArduino();

    bool start() override;
    void stop();

    void emitEvent(const HandlerID_t handlerID) override;

    void dispatchEvents();

protected:
    void notifyDispatcherAboutEvent() override;
    
    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;

    void handleTimers();

private:
    bool mStopDispatcher = false;
    std::map<TimerID_t, RunningTimerInfo> mRunningTimers;
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP
