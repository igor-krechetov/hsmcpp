// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <vector>
#include <map>

namespace hsmcpp
{

class HsmEventDispatcherArduino: public HsmEventDispatcherBase
{
    struct EnqueuedEventInfo
    {
        HandlerID_t handlerID;
        EventID_t eventID;
    };

    struct RunningTimerInfo
    {
        unsigned long startedAt;// time when timer was started (ms)
        unsigned long elapseAfter;// time when timer should elapse next time (ms)
    };

public:
    HsmEventDispatcherArduino(const size_t eventsCacheSize=32);
    virtual ~HsmEventDispatcherArduino();

    bool start() override;
    void stop();

    void emitEvent(const HandlerID_t handlerID) override;
    bool enqueueEvent(const HandlerID_t handlerID, const EventID_t event) override;

    void dispatchEvents();

protected:
    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;

    void handleEnqueuedEvents();
    void handleTimers();

private:
    bool mStopDispatcher = false;
    std::map<TimerID_t, RunningTimerInfo> mRunningTimers;
    std::vector<EnqueuedEventInfo> mEnqueuedEvents;
};

} // namespace hsmcpp

#endif // __HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP__
