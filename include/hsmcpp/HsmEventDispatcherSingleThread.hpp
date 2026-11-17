// Copyright (C) 2026 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERSINGLETHREAD_HPP
#define HSMCPP_HSMEVENTDISPATCHERSINGLETHREAD_HPP

#include <map>

#include "HsmEventDispatcherBase.hpp"

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherSingleThread provides a dispatcher for genuinely single-threaded applications.
 * @details This dispatcher owns no worker thread. The application drives it by calling
 * HsmEventDispatcherSingleThread::dispatchEvents() from its own loop (or periodic callback). All HSM access -- including
 * transitions raised from within state/timer callbacks -- therefore happens on the single thread that calls
 * dispatchEvents(). This is the dispatcher intended to be paired with HSMBUILD_THREAD_SAFETY=OFF: because there is no
 * second thread, the synchronization primitives are pure overhead and may be safely compiled out.
 *
 * It serves as the platform-independent base for HsmEventDispatcherArduino, which only supplies a platform clock.
 * @requirement HSMCPP.SWR_HSM_152
 */
class HsmEventDispatcherSingleThread : public HsmEventDispatcherBase {
protected:
    struct RunningTimerInfo {
        unsigned long startedAt;    ///< monotonic time when timer was started (ms)
        unsigned long elapseAfter;  ///< monotonic time when timer should elapse next time (ms)
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
    static std::shared_ptr<HsmEventDispatcherSingleThread> create(
        const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

    /**
     * @brief See IHsmEventDispatcher::start()
     * @concurrencysafe{ }
     * @requirement HSMCPP.SWR_HSM_140, HSMCPP.SWR_HSM_142
     */
    bool start() override;

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{ }
     */
    void emitEvent(const HandlerID_t handlerID) override;

    /**
     * @brief Dispatch pending events.
     * @details Applications must call this method from their own loop as often as possible. It does nothing and returns
     * quickly if there are no pending events or elapsed timers.
     *
     * @threadsafe{ }
     * @requirement HSMCPP.SWR_HSM_148
     */
    void dispatchEvents();

protected:
    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
     */
    explicit HsmEventDispatcherSingleThread(const size_t eventsCacheSize);

    /**
     * Destructor.
     */
    virtual ~HsmEventDispatcherSingleThread();

    /**
     * @copydoc HsmEventDispatcherBase::deleteSafe()
     */
    bool deleteSafe() override;

    void notifyDispatcherAboutEvent() override;

    /**
     * @brief Current monotonic time in milliseconds.
     * @details Single platform seam. The default implementation uses std::chrono::steady_clock so the dispatcher works
     * out-of-the-box on any hosted platform. Bare-metal/embedded subclasses (e.g. Arduino) override it with their own
     * tick source.
     */
    virtual unsigned long getTickMs() const;

    /**
     * @brief See HsmEventDispatcherBase::startTimerImpl()
     * @interruptsafe{ }
     */
    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    /**
     * @brief See HsmEventDispatcherBase::stopTimerImpl()
     * @interruptsafe{ }
     */
    void stopTimerImpl(const TimerID_t timerID) override;

    void handleTimers();

private:
    std::map<TimerID_t, RunningTimerInfo> mRunningTimers;
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERSINGLETHREAD_HPP
