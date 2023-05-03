// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"

#include <algorithm>

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"

namespace hsmcpp {

constexpr const char* HSM_TRACE_CLASS = "HsmEventDispatcherGLibmm";

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize)
    , mMainContext(Glib::MainContext::get_default()) {
    HSM_TRACE_CALL_DEBUG();
}

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context, const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize)
    , mMainContext(context) {
    HSM_TRACE_CALL_DEBUG();
}

HsmEventDispatcherGLibmm::~HsmEventDispatcherGLibmm() {
    HSM_TRACE_CALL_DEBUG();

    stop();
}

std::shared_ptr<HsmEventDispatcherGLibmm> HsmEventDispatcherGLibmm::create(const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherGLibmm>(new HsmEventDispatcherGLibmm(eventsCacheSize),
                                                     &HsmEventDispatcherBase::handleDelete);
}

std::shared_ptr<HsmEventDispatcherGLibmm> HsmEventDispatcherGLibmm::create(const Glib::RefPtr<Glib::MainContext>& context,
                                                                           const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherGLibmm>(new HsmEventDispatcherGLibmm(context, eventsCacheSize),
                                                     &HsmEventDispatcherBase::handleDelete);
}

bool HsmEventDispatcherGLibmm::deleteSafe() {
    bool deleteNow = false;

    if (mMainContext) {
        mMainContext->signal_idle().connect(sigc::bind(
            [](HsmEventDispatcherGLibmm* pThis) {
                if (nullptr != pThis) {
                    pThis->stop();
                    delete pThis;
                }

                return false;  // unregister idle signal
            },
            this));
    } else {
        deleteNow = true;
    }

    return deleteNow;
}

void HsmEventDispatcherGLibmm::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();

    if (mDispatcherConnection.connected() == true) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

bool HsmEventDispatcherGLibmm::start() {
    bool result = false;

    if (mMainContext) {
        if (!mDispatcher) {
            mDispatcher.reset(new Glib::Dispatcher(mMainContext));

            if (mDispatcher) {
                if (mDispatcherConnection.connected() == false) {
                    mDispatcherConnection =
                        mDispatcher->connect(sigc::mem_fun(this, &HsmEventDispatcherGLibmm::dispatchPendingEvents));
                }

                result = true;
            }
        } else {
            result = true;
        }
    }

    return result;
}

void HsmEventDispatcherGLibmm::stop() {
    HSM_TRACE_CALL_DEBUG();

    HsmEventDispatcherBase::stop();

    if (true == mDispatcherConnection.connected()) {
        mDispatcherConnection.disconnect();
    }

    unregisterAllTimerHandlers();
    unregisterAllEventHandlers();
    mDispatcher.reset();
}

void HsmEventDispatcherGLibmm::unregisterAllTimerHandlers() {
    CriticalSection cs(mRunningTimersSync);

    for (auto it = mNativeTimerHandlers.begin(); it != mNativeTimerHandlers.end(); ++it) {
        it->second.disconnect();
    }

    mNativeTimerHandlers.clear();
}

void HsmEventDispatcherGLibmm::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d",
                              SC2INT(timerID),
                              intervalMs,
                              BOOL2INT(isSingleShot));
    CriticalSection cs(mRunningTimersSync);
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() == it) {
        sigc::connection newTimerConnection;

        newTimerConnection = mMainContext->signal_timeout().connect(
            sigc::bind(sigc::mem_fun(this, &HsmEventDispatcherGLibmm::onTimerEvent), timerID),
            intervalMs);
        mNativeTimerHandlers.emplace(timerID, newTimerConnection);
    } else {
        HSM_TRACE_ERROR("timer with id=%d already exists", timerID);
    }
}

void HsmEventDispatcherGLibmm::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    CriticalSection cs(mRunningTimersSync);
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() != it) {
        it->second.disconnect();
        mNativeTimerHandlers.erase(it);
    }
}

void HsmEventDispatcherGLibmm::notifyDispatcherAboutEvent() {
    mDispatcher->emit();
}

bool HsmEventDispatcherGLibmm::onTimerEvent(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    const bool restartTimer = handleTimerEvent(timerID);

    if (false == restartTimer) {
        CriticalSection cs(mRunningTimersSync);
        auto itTimer = mNativeTimerHandlers.find(timerID);

        if (mNativeTimerHandlers.end() != itTimer) {
            mNativeTimerHandlers.erase(itTimer);
        } else {
            HSM_TRACE_ERROR("unexpected error. timer not found");
        }
    }

    return restartTimer;
}

}  // namespace hsmcpp