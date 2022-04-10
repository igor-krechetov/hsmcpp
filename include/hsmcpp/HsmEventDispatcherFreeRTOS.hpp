// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERFREERTOS_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERFREERTOS_HPP__

#include <vector>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "HsmEventDispatcherBase.hpp"
#include "os/ConditionVariable.hpp"

namespace hsmcpp
{

// NOTE: dispatcher methods should be called only from Tasks. also it should never be deleted from an ISR

class HsmEventDispatcherFreeRTOS: public HsmEventDispatcherBase
{
public:
    struct EnqueuedEventInfo
    {
        HandlerID_t handlerID;
        EventID_t eventID;
    };

    /**
     * Constructor for FreeRTOS based event dispatcher.
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
    HsmEventDispatcherFreeRTOS(const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE,
                               const UBaseType_t priority = tskIDLE_PRIORITY,
                               const size_t eventsCacheSize = 10);
    virtual ~HsmEventDispatcherFreeRTOS();

    virtual void emitEvent(const HandlerID_t handlerID) override;
    virtual bool enqueueEvent(const HandlerID_t handlerID, const EventID_t event) override;

    virtual bool start() override;
    void stop();

    // Blocks current thread until dispatcher is stopped.
    // NOTE: Make sure you call stop() before destroying dispatcher object if you use join() API.
    //       Failing to do so will result in an undefined behaviour.
    void join();

protected:
    virtual void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    virtual void stopTimerImpl(const TimerID_t timerID) override;

    void handleEnqueuedEvents();
    void notifyDispatcherTask();
    static void doDispatching(void* pvParameters);

    static void onTimerEvent(TimerHandle_t timerHandle);
    
private:
    TaskHandle_t mMainTask = nullptr;
    TaskHandle_t mDispatcherTask = nullptr;
    bool mStopDispatcher = false;

    configSTACK_DEPTH_TYPE mStackDepth = configMINIMAL_STACK_SIZE;
    UBaseType_t mPriority = tskIDLE_PRIORITY;
    std::map<TimerID_t, TimerHandle_t> mNativeTimerHandlers;
    std::vector<EnqueuedEventInfo> mEnqueuedEvents;
};

} // namespace hsmcpp

#endif // __HSMCPP_HSMEVENTDISPATCHERFREERTOS_HPP__
