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
        std::chrono::time_point<std::chrono::steady_clock> startedAt;  ///< time when timer was started (monotonic, ms)
        std::chrono::time_point<std::chrono::steady_clock> elapseAfter;  ///< time when timer should elapse next (monotonic , ms)
    };

public:
    /**
     * @brief Create dispatcher instance.
     * @param eventsCacheSize size of the queue preallocated for delayed events
     * @return New dispatcher instance.
     *
     * @threadsafe{Instance can be safely created and destroyed from any thread.}
     */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    static std::shared_ptr<HsmEventDispatcherSTD> create(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

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
     * @copydoc IHsmEventDispatcher::stop()
     * @details Wakes up dispatcher thread and instructs it to stop. Has not effect if thread is not running.
     *
     * @remark Operation is performed asynchronously. Call join() if you need to wait for dispatcher to fully stop.
     *
     * @threadsafe{ }
     */
    void stop() override;

    /**
     * @brief Blocks current thread until dispatcher is stopped.
     * @details This API can be useful if you want to block your main() function until HSM is operable. See
     * ./examples/00_helloworld/00_helloworld_std.cpp for a reference.
     *
     * @remark: Make sure you call stop() before you use join() API.
     */
    void join();

protected:
    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
     */
    explicit HsmEventDispatcherSTD(const size_t eventsCacheSize);

    /**
     * @brief Destructor
     * @details Internally calls stop() and join().
     *
     * @threadsafe{ }
    */
    virtual ~HsmEventDispatcherSTD();

    /**
     * @copydoc HsmEventDispatcherBase::deleteSafe()
     */
    bool deleteSafe() override;
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
    bool mNotifiedTimersThread = false;
    std::map<TimerID_t, RunningTimerInfo> mRunningTimers; // protected by mRunningTimersSync
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERSTD_HPP
