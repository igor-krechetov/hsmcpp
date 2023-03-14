// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERSTD_HPP
#define HSMCPP_HSMEVENTDISPATCHERSTD_HPP

#include <chrono>
#include <map>
#include <thread>

#include "HsmEventDispatcherBase.hpp"
#include "os/ConditionVariable.hpp"

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherSTD provides platform independent dispatcher implementation based on standard C++ library.
 * @details See @rstref{platforms-dispatcher-std} for details.
 */
class HsmEventDispatcherSTD : public HsmEventDispatcherBase {
private:
    struct RunningTimerInfo {
        std::chrono::time_point<std::chrono::steady_clock> startedAt;  ///< monotonic time when timer was started (ms)
        std::chrono::time_point<std::chrono::steady_clock>
            elapseAfter;  ///< monotonic time when timer should elapse next time (ms)
    };

public:
    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
    */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    explicit HsmEventDispatcherSTD(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);
    virtual ~HsmEventDispatcherSTD();

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{ }
     */
    void emitEvent(const HandlerID_t handlerID) override;

    /**
     * @copydoc IHsmEventDispatcher::start()
     * @details Function starts a new std::thread for dispatching events. Thread can be stopped by calling stop().
     *
     * @notthreadsafe{This API is intended to be called only once during startup.}
    */
    bool start() override;

    /**
     * @brief Stop events dispatcher thread.
     * @details Wakes up dispatcher thread and instructs it to stop. Has not effect if thread is not running.
     *
     * @remark Operation is performed asynchronously.
    */
    void stop();

    /**
     * @brief Blocks current thread until dispatcher is stopped.
     * @details This API can be useful if you want to block your main() function until HSM is operable. See
     * ./examples/00_helloworld/00_helloworld_std.cpp for a reference.
     *
     * @remark: Make sure you call stop() before destroying dispatcher object if you use join() API. Failing to do so will
     * result in an undefined behaviour.
     */
    void join();

protected:
    /**
     * @brief See HsmEventDispatcherBase::startTimerImpl()
     * @threadsafe{ }
    */
    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    /**
     * @brief See HsmEventDispatcherBase::stopTimerImpl()
     * @threadsafe{ }
    */
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

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERSTD_HPP
