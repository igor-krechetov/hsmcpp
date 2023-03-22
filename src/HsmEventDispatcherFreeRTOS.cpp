// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/HsmEventDispatcherFreeRTOS.hpp"

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/InterruptsFreeSection.hpp"
#include "hsmcpp/os/freertos/FreeRtosPort.hpp"

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
//       - need to test if it's safe to lock a Mutex inside ISR. Then it would be better
//         to replace InterruptsFreeSection with CriticalSection

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherFreeRTOS"

HsmEventDispatcherFreeRTOS::HsmEventDispatcherFreeRTOS(const configSTACK_DEPTH_TYPE stackDepth,
                                                       const UBaseType_t priority,
                                                       const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize)
    , mStackDepth(stackDepth)
    , mPriority(priority) {
    HSM_TRACE_CALL_DEBUG_ARGS("stackDepth=%d, priority=%d", static_cast<int>(stackDepth), static_cast<int>(priority));
}

HsmEventDispatcherFreeRTOS::~HsmEventDispatcherFreeRTOS() {
    HSM_TRACE_CALL_DEBUG();

    stop();
    join();
}

bool HsmEventDispatcherFreeRTOS::deleteSafe() {
    // NOTE: just delete the instance. Calling destructor from any thread is safe
    return true;
}

std::shared_ptr<HsmEventDispatcherFreeRTOS> HsmEventDispatcherFreeRTOS::create(const configSTACK_DEPTH_TYPE stackDepth,
                                                                               const UBaseType_t priority,
                                                                               const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherFreeRTOS>(new HsmEventDispatcherFreeRTOS(stackDepth, priority, eventsCacheSize),
                                                       &HsmEventDispatcherBase::handleDelete);
}

void HsmEventDispatcherFreeRTOS::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();

    if (nullptr != mDispatcherTask) {
        dispatchEnqueuedEvents();

        {
            // TODO: this will work only for single-core FreeRTOS
            InterruptsFreeSection lck;
            mPendingEvents.push_back(handlerID);
        }

        notifyDispatcherAboutEvent();
    }
}

bool HsmEventDispatcherFreeRTOS::start() {
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    if (nullptr == mDispatcherTask) {
        mMainTask = xTaskGetCurrentTaskHandle();

        if (nullptr != mMainTask) {
            HSM_TRACE_DEBUG("starting task...");
            BaseType_t taskStatus = xTaskCreate(HsmEventDispatcherFreeRTOS::doDispatching,
                                                "HsmEventDispatcherFreeRTOS",
                                                mStackDepth,
                                                this,
                                                mPriority,
                                                &mDispatcherTask);

            if (pdPASS == taskStatus) {
                mStopDispatcher = false;
                result = true;
            } else {
                HSM_TRACE_ERROR("failed to start task");
            }
        } else {
            HSM_TRACE_ERROR("dispatcher must be started from a Task");
        }
    } else {
        // just make sure that task was not requested to stop
        result = (false == mStopDispatcher);
    }

    return result;
}

void HsmEventDispatcherFreeRTOS::stop() {
    HSM_TRACE_CALL_DEBUG();

    HsmEventDispatcherBase::stop();

    unregisterAllEventHandlers();

    if (nullptr != mDispatcherTask) {
        notifyDispatcherAboutEvent();
    }

    // delete timers
    for (auto it = mNativeTimerHandlers.begin(); it != mNativeTimerHandlers.end(); ++it) {
        xTimerDelete(it->second, 0);
    }
    mNativeTimerHandlers.clear();
}

void HsmEventDispatcherFreeRTOS::join() {
    HSM_TRACE_CALL_DEBUG();

    if (nullptr != mDispatcherTask) {
        vTaskSuspend(nullptr);
    }
}

void HsmEventDispatcherFreeRTOS::startTimerImpl(const TimerID_t timerID,
                                                const unsigned int intervalMs,
                                                const bool isSingleShot) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d",
                              SC2INT(timerID),
                              intervalMs,
                              BOOL2INT(isSingleShot));
    auto it = mNativeTimerHandlers.end();
    TimerHandle_t timer = nullptr;
    UBaseType_t reloadMode = (false == isSingleShot ? pdTRUE : pdFALSE);

    {
        InterruptsFreeSection lck;
        it = mNativeTimerHandlers.find(timerID);
    }

    if (mNativeTimerHandlers.end() == it) {
        timer = xTimerCreate("hsmtimer",
                             intervalMs / portTICK_PERIOD_MS,
                             reloadMode,
                             this,
                             HsmEventDispatcherFreeRTOS::onTimerEvent);

        if (nullptr != timer) {
            InterruptsFreeSection lck;
            mNativeTimerHandlers.emplace(timerID, timer);
        }
    } else {
        timer = it->second;

        if ((xTimerGetPeriod(timer) * portTICK_PERIOD_MS) != intervalMs) {
            if (pdPASS != xTimerChangePeriod(timer, intervalMs / portTICK_PERIOD_MS, 0)) {
                xTimerDelete(timer, 0);
                {
                    InterruptsFreeSection lck;
                    mNativeTimerHandlers.erase(timerID);
                }

                HSM_TRACE_ERROR("xTimerChangePeriod failed");
            }
        }

        // NOTE: not available in all versions
        // if (uxTimerGetReloadMode(timer) != reloadMode)
        { vTimerSetReloadMode(timer, reloadMode); }
    }

    if (nullptr != timer) {
        // try to start timer without blocking
        // NOTE: this will not work for sync transitions which are called from ISR
        if (pdPASS != xTimerStart(timer, 0)) {
            xTimerDelete(timer, 0);
            {
                InterruptsFreeSection lck;
                mNativeTimerHandlers.erase(timerID);
            }

            HSM_TRACE_ERROR("xTimerStart failed");
        }
    } else {
        HSM_TRACE_ERROR("xTimerCreate failed");
    }
}

void HsmEventDispatcherFreeRTOS::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    auto it = mNativeTimerHandlers.end();

    {
        InterruptsFreeSection lck;
        it = mNativeTimerHandlers.find(timerID);
    }

    if (mNativeTimerHandlers.end() != it) {
        // NOTE: this will not work for sync transitions which are called from ISR
        xTimerStop(it->second, 0);  // NOTE: this will not be processed instantly and can actually fail
    }
}

void HsmEventDispatcherFreeRTOS::notifyDispatcherAboutEvent() {
    if (nullptr != mDispatcherTask) {
        BaseType_t isInsideISR = xPortIsInsideInterrupt();

        if (pdFALSE == isInsideISR) {
            xTaskNotifyGive(mDispatcherTask);
        } else {
            vTaskNotifyGiveFromISR(mDispatcherTask, nullptr);
        }
    }
}

void HsmEventDispatcherFreeRTOS::doDispatching(void* pvParameters) {
    HSM_TRACE_CALL_DEBUG();
    HsmEventDispatcherFreeRTOS* pThis = static_cast<HsmEventDispatcherFreeRTOS*>(pvParameters);

    if (nullptr != pThis) {
        while (false == pThis->mStopDispatcher) {
            std::list<HandlerID_t> events;

            {
                // NOTE: this will work only on a single core implementation of FreeRTOS
                //       for multi-core system mutex lock will be required
                InterruptsFreeSection lck;
                events = std::move(pThis->mPendingEvents);
            }

            pThis->dispatchPendingEventsImpl(events);

            if (false == pThis->mStopDispatcher) {
                if (true == pThis->mPendingEvents.empty()) {
                    HSM_TRACE_DEBUG("wait for emit...");
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    HSM_TRACE_DEBUG("woke up. pending events=%lu", pThis->mPendingEvents.size());
                }
            }
        }
    }

    HSM_TRACE_DEBUG("EXIT");

    if (nullptr != pThis->mMainTask) {
        if (eSuspended == eTaskGetState(pThis->mMainTask)) {
            vTaskResume(pThis->mMainTask);
        }
    }

    pThis->mDispatcherTask = nullptr;
    vTaskDelete(nullptr);
}

void HsmEventDispatcherFreeRTOS::onTimerEvent(TimerHandle_t timerHandle) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerHandle=%p", timerHandle);

    if (nullptr != timerHandle) {
        HsmEventDispatcherFreeRTOS* pThis = static_cast<HsmEventDispatcherFreeRTOS*>(pvTimerGetTimerID(timerHandle));

        if (nullptr != pThis) {
            TimerID_t timerID = INVALID_HSM_TIMER_ID;

            {
                InterruptsFreeSection lck;

                for (auto it = pThis->mNativeTimerHandlers.begin(); it != pThis->mNativeTimerHandlers.end(); ++it) {
                    if (it->second == timerHandle) {
                        timerID = it->first;
                        break;
                    }
                }
            }

            const bool restartTimer = pThis->handleTimerEvent(timerID);

            HSM_TRACE_DEBUG("restartTimer=%d", (int)restartTimer);

            if (false == restartTimer) {
                InterruptsFreeSection lck;
                auto itTimer = pThis->mNativeTimerHandlers.find(timerID);

                if (pThis->mNativeTimerHandlers.end() != itTimer) {
                    pThis->mNativeTimerHandlers.erase(itTimer);
                } else {
                    HSM_TRACE_ERROR("unexpected error. timer not found");
                }
            }
        }
    }
}

}  // namespace hsmcpp