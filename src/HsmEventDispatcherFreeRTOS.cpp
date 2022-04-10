// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/HsmEventDispatcherFreeRTOS.hpp"
#include "hsmcpp/os/CriticalSection.hpp"
#include "hsmcpp/os/freertos/FreeRtosPort.hpp"
#include "hsmcpp/logging.hpp"

#if (INCLUDE_xTaskGetCurrentTaskHandle != 1)
 #error INCLUDE_xTaskGetCurrentTaskHandle feature is required
#endif
#if (configUSE_TASK_NOTIFICATIONS != 1)
  #error configUSE_TASK_NOTIFICATIONS feature is required
#endif
#if (configUSE_TIMERS != 1)
  #error configUSE_TIMERS feature is required
#endif

// TODO: supports only single core implementation of FreeRTOS. For multi-core systems need to clarify:
//       - AMP or SMP is used?
//       - can multiple ISR be called at the same time?

namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherFreeRTOS"

HsmEventDispatcherFreeRTOS::HsmEventDispatcherFreeRTOS(const configSTACK_DEPTH_TYPE stackDepth, const UBaseType_t priority, const size_t eventsCacheSize)
    : mStackDepth(stackDepth)
    , mPriority(priority)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("stackDepth=%d, priority=%d", static_cast<int>(stackDepth), static_cast<int>(priority));

    mEnqueuedEvents.reserve(eventsCacheSize);
}

HsmEventDispatcherFreeRTOS::~HsmEventDispatcherFreeRTOS()
{
    __HSM_TRACE_CALL_DEBUG__();

    // delete timers
    for (auto it = mNativeTimerHandlers.begin(); it != mNativeTimerHandlers.end(); ++it)
    {
        xTimerDelete(it->second, 0);
    }
    mNativeTimerHandlers.clear();

    unregisterAllEventHandlers();
    stop();
    join();
}

void HsmEventDispatcherFreeRTOS::emitEvent(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG__();

    if (nullptr != mDispatcherTask)
    {
        handleEnqueuedEvents();

        {
            // TODO: this will work only for single-core FreeRTOS
            CriticalSection lck;
            mPendingEvents.push_back(handlerID);
        }

        notifyDispatcherTask();
    }
}

bool HsmEventDispatcherFreeRTOS::enqueueEvent(const HandlerID_t handlerID, const EventID_t event)
{
    bool wasAdded = false;

    if (mEnqueuedEvents.size() < mEnqueuedEvents.capacity())
    {
        EnqueuedEventInfo newEvent;

        newEvent.handlerID = handlerID;
        newEvent.eventID = event;
        mEnqueuedEvents.push_back(newEvent);
        wasAdded = true;

        notifyDispatcherTask();
    }

    return wasAdded;
}

bool HsmEventDispatcherFreeRTOS::start()
{
    __HSM_TRACE_CALL_DEBUG__();
    bool result = false;

    if (nullptr == mDispatcherTask)
    {
        mMainTask = xTaskGetCurrentTaskHandle();

        if (nullptr != mMainTask)
        {
            __HSM_TRACE_DEBUG__("starting task...");
            BaseType_t taskStatus = xTaskCreate(HsmEventDispatcherFreeRTOS::doDispatching,
                                                "HsmEventDispatcherFreeRTOS", 
                                                mStackDepth, 
                                                this, 
                                                mPriority, 
                                                &mDispatcherTask);

            if (pdPASS == taskStatus)
            {
                mStopDispatcher = false;
                result = true;
            }
            else
            {
                __HSM_TRACE_ERROR__("failed to start task");
            }
        }
        else
        {
            __HSM_TRACE_ERROR__("dispatcher must be started from a Task");
        }
    }
    else
    {
        // just make sure that task was not requested to stop
        result = (false == mStopDispatcher);
    }

    return result;
}

void HsmEventDispatcherFreeRTOS::stop()
{
    __HSM_TRACE_CALL_DEBUG__();

    if (nullptr != mDispatcherTask)
    {
        mStopDispatcher = true;
        notifyDispatcherTask();
    }
}

void HsmEventDispatcherFreeRTOS::join()
{
    __HSM_TRACE_CALL_DEBUG__();

    if (nullptr != mDispatcherTask)
    {
        vTaskSuspend(nullptr);
    }
}

void HsmEventDispatcherFreeRTOS::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d, intervalMs=%d, isSingleShot=%d",
                                  SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));
    auto it = mNativeTimerHandlers.end();
    TimerHandle_t timer = nullptr;
    UBaseType_t reloadMode = (false == isSingleShot ? pdTRUE : pdFALSE);

    {
        CriticalSection lck;
        it = mNativeTimerHandlers.find(timerID);
    }

    if (mNativeTimerHandlers.end() == it)
    {
        timer = xTimerCreate("hsmtimer",
                             intervalMs / portTICK_PERIOD_MS,
                             reloadMode,
                             this,
                             HsmEventDispatcherFreeRTOS::onTimerEvent);

        if (nullptr != timer)
        {
            CriticalSection lck;
            mNativeTimerHandlers.emplace(timerID, timer);
        }
    }
    else
    {
        timer = it->second;

        if ((xTimerGetPeriod(timer) * portTICK_PERIOD_MS) != intervalMs)
        {
            if (pdPASS != xTimerChangePeriod(timer, intervalMs / portTICK_PERIOD_MS, 0))
            {
                xTimerDelete(timer, 0);
                {
                    CriticalSection lck;
                    mNativeTimerHandlers.erase(timerID);
                }

                __HSM_TRACE_ERROR__("xTimerChangePeriod failed");
            }
        }

        // NOTE: not available in all versions
        // if (uxTimerGetReloadMode(timer) != reloadMode)
        {
            vTimerSetReloadMode(timer, reloadMode);
        }
    }

    if (nullptr != timer)
    {
        // try to start timer without blocking
        // NOTE: this will not work for sync transitions which are called from ISR
        if (pdPASS != xTimerStart(timer, 0))
        {
            xTimerDelete(timer, 0);
            {
                CriticalSection lck;
                mNativeTimerHandlers.erase(timerID);
            }

            __HSM_TRACE_ERROR__("xTimerStart failed");
        }
    }
    else
    {
        __HSM_TRACE_ERROR__("xTimerCreate failed");
    }
}

void HsmEventDispatcherFreeRTOS::stopTimerImpl(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    auto it = mNativeTimerHandlers.end();

    {
        CriticalSection lck;
        it = mNativeTimerHandlers.find(timerID);
    }

    if (mNativeTimerHandlers.end() != it)
    {
        // NOTE: this will not work for sync transitions which are called from ISR
        xTimerStop(it->second, 0);// NOTE: this will not be processed instantly and can actually fail
    }
}

void HsmEventDispatcherFreeRTOS::handleEnqueuedEvents()
{
    if (mEnqueuedEvents.size() > 0)
    {
        HandlerID_t prevHandlerID = INVALID_HSM_DISPATCHER_HANDLER_ID;
        EnqueuedEventHandlerFunc_t callback;
        std::vector<EnqueuedEventInfo> currentEvents;

        {
            CriticalSection lck;
        
            currentEvents = mEnqueuedEvents;
            mEnqueuedEvents.clear();
        }

        // need to traverse events in reverce order 
        for (auto it = currentEvents.rbegin(); it != currentEvents.rend(); ++it)
        {
            if (prevHandlerID != it->handlerID)
            {
                callback = getEnqueuedEventHandlerFunc(it->handlerID);
                prevHandlerID = it->handlerID;
            }

            callback(it->eventID);
        }
    }
}

void HsmEventDispatcherFreeRTOS::notifyDispatcherTask()
{
    if (nullptr != mDispatcherTask)
    {
        BaseType_t isInsideISR = xPortIsInsideInterrupt();

        if (pdFALSE == isInsideISR)
        {
            xTaskNotifyGive(mDispatcherTask);
        }
        else
        {
            vTaskNotifyGiveFromISR(mDispatcherTask, nullptr);
        }
    }
}

void HsmEventDispatcherFreeRTOS::doDispatching(void* pvParameters)
{
    __HSM_TRACE_CALL_DEBUG__();
    HsmEventDispatcherFreeRTOS* pThis = static_cast<HsmEventDispatcherFreeRTOS*>(pvParameters);

    if (nullptr != pThis)
    {
        while (false == pThis->mStopDispatcher)
        {
            std::list<HandlerID_t> events;

            pThis->handleEnqueuedEvents();

            {
                // NOTE: this will work only on a single core implementation of FreeRTOS
                //       for multi-core system mutex lock will be required
                CriticalSection lck;
                events = std::move(pThis->mPendingEvents);
            }

            pThis->dispatchPendingEvents(events);

            if (false == pThis->mStopDispatcher)
            {
                if (true == pThis->mPendingEvents.empty())
                {
                    __HSM_TRACE_DEBUG__("wait for emit...");
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    __HSM_TRACE_DEBUG__("woke up. pending events=%lu", pThis->mPendingEvents.size());
                }
            }
        }
    }

    __HSM_TRACE_DEBUG__("EXIT");

    if (nullptr != pThis->mMainTask)
    {
        if (eSuspended == eTaskGetState(pThis->mMainTask))
        {
            vTaskResume(pThis->mMainTask);
        }
    }

    pThis->mDispatcherTask = nullptr;
    vTaskDelete(nullptr);
}

void HsmEventDispatcherFreeRTOS::onTimerEvent(TimerHandle_t timerHandle)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerHandle=%p", timerHandle);

    if (nullptr != timerHandle)
    {
        HsmEventDispatcherFreeRTOS* pThis = static_cast<HsmEventDispatcherFreeRTOS*>(pvTimerGetTimerID(timerHandle));

        if (nullptr != pThis)
        {
            TimerID_t timerID = INVALID_HSM_TIMER_ID;

            {
                CriticalSection lck;

                for (auto it = pThis->mNativeTimerHandlers.begin(); it != pThis->mNativeTimerHandlers.end(); ++it)
                {
                    if (it->second == timerHandle)
                    {
                        timerID = it->first;
                        break;
                    }
                }
            }

            const bool restartTimer = pThis->handleTimerEvent(timerID);

            __HSM_TRACE_DEBUG__("restartTimer=%d", (int)restartTimer);

            if (false == restartTimer)
            {
                CriticalSection lck;
                auto itTimer = pThis->mNativeTimerHandlers.find(timerID);

                if (pThis->mNativeTimerHandlers.end() != itTimer)
                {
                    pThis->mNativeTimerHandlers.erase(itTimer);
                }
                else
                {
                    __HSM_TRACE_ERROR__("unexpected error. timer not found");
                }
            }
        }
    }
}

}