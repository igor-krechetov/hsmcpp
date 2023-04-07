// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherGLib.hpp"

#include <unistd.h>

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherGLib"

HsmEventDispatcherGLib::HsmEventDispatcherGLib(const size_t eventsCacheSize)
    // NOTE: false-positive. thinks that ':' is arithmetic operation
    // cppcheck-suppress misra-c2012-10.4
    : HsmEventDispatcherBase(eventsCacheSize) {}

HsmEventDispatcherGLib::HsmEventDispatcherGLib(GMainContext* context, const size_t eventsCacheSize)
    // NOTE: false-positive. thinks that ':' is arithmetic operation
    // cppcheck-suppress misra-c2012-10.4
    : HsmEventDispatcherBase(eventsCacheSize)
    , mContext(context) {}

HsmEventDispatcherGLib::~HsmEventDispatcherGLib() {
    HSM_TRACE_CALL();

    HsmEventDispatcherGLib::stop();
}

std::shared_ptr<HsmEventDispatcherGLib> HsmEventDispatcherGLib::create(const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherGLib>(new HsmEventDispatcherGLib(eventsCacheSize),
                                                   &HsmEventDispatcherBase::handleDelete);
}

std::shared_ptr<HsmEventDispatcherGLib> HsmEventDispatcherGLib::create(GMainContext* context, const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherGLib>(new HsmEventDispatcherGLib(context, eventsCacheSize),
                                                   &HsmEventDispatcherBase::handleDelete);
}

bool HsmEventDispatcherGLib::deleteSafe() {
    g_idle_add(
        // cppcheck-suppress misra-c2012-13.1 ; false-positive. this is a functor, not initializer list
        [](void* data) {
            if (nullptr != data) {
                HsmEventDispatcherGLib* pThis = reinterpret_cast<HsmEventDispatcherGLib*>(data);

                pThis->stop();
                delete pThis;
            }

            // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
            // cppcheck-suppress misra-c2012-15.5
            return G_SOURCE_REMOVE;
        },
        this);

    return false;
}

bool HsmEventDispatcherGLib::start() {
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    // check if dispatcher was already started
    if ((-1) == mPipeFD[0]) {
        int rc = pipe(mPipeFD);

        if (0 == rc) {
            mReadChannel = g_io_channel_unix_new(mPipeFD[0]);

            if (nullptr != mReadChannel) {
                mIoSource = g_io_create_watch(mReadChannel, static_cast<GIOCondition>(G_IO_IN | G_IO_HUP));

                if (nullptr != mIoSource) {
                    g_source_set_callback(mIoSource, reinterpret_cast<GSourceFunc>(onPipeDataAvailable), this, nullptr);
                    g_source_attach(mIoSource, mContext);
                    result = true;
                } else {
                    HSM_TRACE_ERROR("failed to create io source");
                }
            } else {
                HSM_TRACE_ERROR("failed to create io channel");
            }
        } else {
            HSM_TRACE_ERROR("failed to create pipe (errno=%d)", errno);
        }

        if (false == result) {
            if (nullptr != mReadChannel) {
                g_io_channel_unref(mReadChannel);
                mReadChannel = nullptr;
            }

            if ((-1) == mPipeFD[0]) {
                close(mPipeFD[0]);
                close(mPipeFD[1]);
                mPipeFD[0] = -1;
                mPipeFD[1] = -1;
            }
        }
    } else {
        result = true;
    }

    return result;
}

void HsmEventDispatcherGLib::stop() {
    HsmEventDispatcherBase::stop();
    unregisterAllTimerHandlers();
    unregisterAllEventHandlers();

    if (nullptr != mIoSource) {
        g_source_destroy(mIoSource);
        g_source_unref(mIoSource);
        mIoSource = nullptr;
    }

    if (nullptr != mReadChannel) {
        g_io_channel_unref(mReadChannel);
        mReadChannel = nullptr;
        close(mPipeFD[0]);
        close(mPipeFD[1]);
        mPipeFD[0] = -1;
        mPipeFD[1] = -1;
    }
}

void HsmEventDispatcherGLib::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL();

    if (mPipeFD[1] > 0) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

void HsmEventDispatcherGLib::unregisterAllTimerHandlers() {
    CriticalSection lckExpired(mRunningTimersSync);

    for (auto it = mNativeTimerHandlers.begin(); it != mNativeTimerHandlers.end(); ++it) {
        g_source_destroy(it->second);
        g_source_unref(it->second);
    }

    mNativeTimerHandlers.clear();
}

void HsmEventDispatcherGLib::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d",
                              SC2INT(timerID),
                              intervalMs,
                              BOOL2INT(isSingleShot));
    CriticalSection lckExpired(mRunningTimersSync);
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() == it) {
        GSource* timeoutSource = g_timeout_source_new(intervalMs);

        g_source_set_callback(timeoutSource,
                              G_SOURCE_FUNC(&HsmEventDispatcherGLib::onTimerEvent),
                              new TimerData_t(this, timerID),
                              &HsmEventDispatcherGLib::onFreeTimerData);

        g_source_attach(timeoutSource, mContext);
        mNativeTimerHandlers.emplace(timerID, timeoutSource);
    } else {
        HSM_TRACE_ERROR("timer with id=%d already exists", timerID);
    }
}

void HsmEventDispatcherGLib::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    CriticalSection lckExpired(mRunningTimersSync);
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() != it) {
        g_source_destroy(it->second);
        g_source_unref(it->second);
        mNativeTimerHandlers.erase(it);
    }
}

gboolean HsmEventDispatcherGLib::onTimerEvent(const TimerData_t* timerData) {
    bool restartTimer = false;

    if (nullptr != timerData) {
        restartTimer = timerData->first->handleTimerEvent(timerData->second);

        if (false == restartTimer) {
            CriticalSection lckExpired(timerData->first->mRunningTimersSync);
            auto itTimer = timerData->first->mNativeTimerHandlers.find(timerData->second);

            if (timerData->first->mNativeTimerHandlers.end() != itTimer) {
                g_source_unref(itTimer->second);
                timerData->first->mNativeTimerHandlers.erase(itTimer);
            } else {
                HSM_TRACE_DEF();
                HSM_TRACE_ERROR("unexpected error. timer not found");
            }
        }
    }

    return restartTimer;
}

void HsmEventDispatcherGLib::onFreeTimerData(void* timerData) {
    delete reinterpret_cast<TimerData_t*>(timerData);
}

void HsmEventDispatcherGLib::notifyDispatcherAboutEvent() {
    std::lock_guard<std::mutex> lck(mPipeSync);
    char dummy = 1;

    // we just need to trigger the callback. there is no need to pass any real data there
    // TODO: should we check for result?
    write(mPipeFD[1], &dummy, sizeof(dummy));
}

gboolean HsmEventDispatcherGLib::onPipeDataAvailable(GIOChannel* gio, GIOCondition condition, gpointer data) {
    HSM_TRACE_CALL();
    gboolean continueDispatching = TRUE;
    HsmEventDispatcherGLib* pThis = static_cast<HsmEventDispatcherGLib*>(data);

    if (false == pThis->mStopDispatcher) {
        pThis->mDispatchingIterationRunning = true;

        HSM_TRACE_DEBUG("condition=%d, G_IO_HUP=%s, G_IO_IN=%s",
                        static_cast<int>(condition),
                        BOOL2STR(condition & G_IO_HUP),
                        BOOL2STR(condition & G_IO_IN));

        if (!(condition & G_IO_HUP)) {
            char dummy;
            int bytes = read(pThis->mPipeFD[0], &dummy, sizeof(dummy));

            if (1 == bytes) {
                pThis->dispatchPendingEvents();
            }
        }

        if (true == pThis->mStopDispatcher) {
            continueDispatching = FALSE;
            pThis->mDispatchingDoneEvent.notify_one();
        }

        pThis->mDispatchingIterationRunning = false;
    } else {
        continueDispatching = FALSE;
    }

    return continueDispatching;
}

}  // namespace hsmcpp