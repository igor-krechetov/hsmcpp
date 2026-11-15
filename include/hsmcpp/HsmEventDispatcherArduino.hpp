// Copyright (C) 2022 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP
#define HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP

#include "os/os.hpp"

#ifndef HSM_DISABLE_THREADSAFETY
#error "HsmEventDispatcherArduino requires thread-safety support to be disabled (Arduino has no threads). "\
       "Build hsmcpp with HSMBUILD_THREAD_SAFETY=OFF or define HSM_DISABLE_THREADSAFETY."
#endif

#include "HsmEventDispatcherSingleThread.hpp"

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherArduino provides dispatcher for Arduino platform.
 * @details Clients are supposed to call HsmEventDispatcherArduino::dispatchEvents() in loop() method of the application.
 * See @rstref{platforms-dispatcher-arduino} for details.
 *
 * It is a thin specialization of HsmEventDispatcherSingleThread: the only Arduino-specific behavior is the monotonic
 * clock source (millis()); all queue and timer logic is inherited from the base.
 * @requirement HSMCPP.SWR_HSM_110
 */
class HsmEventDispatcherArduino : public HsmEventDispatcherSingleThread {
public:
    /**
     * @brief Create dispatcher instance.
     * @param eventsCacheSize size of the queue preallocated for delayed events
     * @return New dispatcher instance.
     *
     * @threadsafe{Instance can be safely created and destroyed from any thread.}
     */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    static std::shared_ptr<HsmEventDispatcherArduino> create(
        const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

protected:
    /**
     * @copydoc HsmEventDispatcherSingleThread::HsmEventDispatcherSingleThread()
     */
    explicit HsmEventDispatcherArduino(const size_t eventsCacheSize);

    /**
     * Destructor.
     */
    virtual ~HsmEventDispatcherArduino();

    /**
     * @brief Monotonic time in milliseconds, provided by Arduino's millis().
     * @copydoc HsmEventDispatcherSingleThread::getTickMs()
     */
    unsigned long getTickMs() const override;
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERARDUINO_HPP
