// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERFREERTOS_HPP
#define HSMCPP_HSMEVENTDISPATCHERFREERTOS_HPP

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <vector>

#include "HsmEventDispatcherBase.hpp"
#include "os/ConditionVariable.hpp"

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherFreeRTOS provides dispatcher implementation for FreeRTOS platform.
 * @details Events dispatching is done using a custom Task. See @rstref{platforms-dispatcher-freertos} for details.
 *
 * @warning Dispatcher methods should be called only from Tasks. Dispatcher instance should never be deleted from an ISR.
 */
class HsmEventDispatcherFreeRTOS : public HsmEventDispatcherBase {
public:
    /**
     * @brief Create dispatcher instance.
     *
     * @param stackDepth The number of words (not bytes!) to allocate for use as the task's stack.
     *                   For example, if the stack is 16-bits wide and usStackDepth is 100, then
     *                   200 bytes will be allocated for use as the task's stack. As another example,
     *                   if the stack is 32-bits wide and usStackDepth is 400 then 1600 bytes will
     *                   be allocated for use as the task's stack.
     *                   The stack depth multiplied by the stack width must not exceed the maximum
     *                   value that can be contained in a variable of type size_t.
     * @param priority   The priority at which the created task will execute.
     *                   Systems that include MPU support can optionally create a task in a
     *                   privileged (system) mode by setting the bit portPRIVILEGE_BIT in uxPriority.
     *                   For example, to create a privileged task at priority 2 set uxPriority to ( 2 | portPRIVILEGE_BIT ).
     *                   Priorities are asserted to be less than configMAX_PRIORITIES. If configASSERT is
     *                   undefined, priorities are silently capped at (configMAX_PRIORITIES - 1).
     * @param eventsCacheSize size of the queue preallocated for delayed events
     */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    static std::shared_ptr<HsmEventDispatcherFreeRTOS> create(
        const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE,
        const UBaseType_t priority = tskIDLE_PRIORITY,
        const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{Only for systems with single core CPU}
     */
    void emitEvent(const HandlerID_t handlerID) override;

    /**
     * @brief See IHsmEventDispatcher::start()
     * @notthreadsafe{Thread safety is not required by HierarchicalStateMachine::initialize() which uses this API.}
     */
    bool start() override;

    /**
     * @copydoc IHsmEventDispatcher::stop()
     * @details Wakes up dispatcher Task and instructs it to stop. Has no effect if Task is not running.
     */
    void stop();

    /**
     * @brief Blocks current thread until dispatcher is stopped.
     * @details Make sure you call stop() before destroying dispatcher object if you use join() API. Failing to do so will
     * result in an undefined behaviour.
     */
    void join();

protected:
    /**
     * @brief Constructor for FreeRTOS based event dispatcher.
     *
     */
    HsmEventDispatcherFreeRTOS(const configSTACK_DEPTH_TYPE stackDepth,
                               const UBaseType_t priority,
                               const size_t eventsCacheSize);

    /**
     * Destructor.
     */
    virtual ~HsmEventDispatcherFreeRTOS();

    bool deleteSafe() override;

    virtual void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    virtual void stopTimerImpl(const TimerID_t timerID) override;

    void notifyDispatcherAboutEvent() override;
    static void doDispatching(void* pvParameters);

    static void onTimerEvent(TimerHandle_t timerHandle);

private:
    TaskHandle_t mMainTask = nullptr;
    TaskHandle_t mDispatcherTask = nullptr;

    configSTACK_DEPTH_TYPE mStackDepth = configMINIMAL_STACK_SIZE;
    UBaseType_t mPriority = tskIDLE_PRIORITY;
    std::map<TimerID_t, TimerHandle_t> mNativeTimerHandlers;
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERFREERTOS_HPP
