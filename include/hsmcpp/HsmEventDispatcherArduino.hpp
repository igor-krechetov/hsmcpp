// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP
#define HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP

#include <map>
#include <vector>

#include "HsmEventDispatcherBase.hpp"

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherArduino provides dispatcher for Arduino platform.
 * @details Clients are supposed to call HsmEventDispatcherArduino::dispatchEvents() in loop() method of the application.
 * See @rstref{platforms-dispatcher-arduino} for details.
 */
class HsmEventDispatcherArduino : public HsmEventDispatcherBase {
private:
    struct RunningTimerInfo {
        unsigned long startedAt;    // monotonic time when timer was started (ms)
        unsigned long elapseAfter;  // monotonic time when timer should elapse next time (ms)
    };

public:
    /**
     * @brief Create dispatcher instance.
     *
     * @threadsafe{Instance can be safely created and destroyed from any thread.}
     */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    static std::shared_ptr<HsmEventDispatcherArduino> create(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

    /**
     * @brief See IHsmEventDispatcher::start()
     * @concurrencysafe{ }
     */
    bool start() override;

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{ }
     */
    void emitEvent(const HandlerID_t handlerID) override;

    /**
     * @brief Dispatch pending events.
     * @details Applications must call this method in Android's loop() function as often as possible. It does nothing and
     * returns quickly if there are no pending events.
     *
     * @threadsafe{ }
     */
    void dispatchEvents();

protected:
    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
     */
    explicit HsmEventDispatcherArduino(const size_t eventsCacheSize);

    /**
     * Destructor.
     */
    virtual ~HsmEventDispatcherArduino();

    bool deleteSafe() override;

    void notifyDispatcherAboutEvent() override;

    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;

    void handleTimers();

private:
    std::map<TimerID_t, RunningTimerInfo> mRunningTimers;
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP
