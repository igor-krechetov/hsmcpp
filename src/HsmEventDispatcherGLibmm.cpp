// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"

#include <algorithm>

#include "hsmcpp/logging.hpp"

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherGLibmm"

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize)
    , mMainContext(Glib::MainContext::get_default())
    , mDispatcher(new Glib::Dispatcher()) {
    HSM_TRACE_CALL_DEBUG();
}

HsmEventDispatcherGLibmm::HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context, const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize)
    , mMainContext(context)
    , mDispatcher(new Glib::Dispatcher(context)) {
    HSM_TRACE_CALL_DEBUG();
}

HsmEventDispatcherGLibmm::~HsmEventDispatcherGLibmm() {
    HSM_TRACE_CALL_DEBUG();

    if (true == mDispatcherConnection.connected()) {
        mDispatcherConnection.disconnect();
    }

    unregisterAllTimerHandlers();
    unregisterAllEventHandlers();
}

void HsmEventDispatcherGLibmm::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();
    if (mDispatcher) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

bool HsmEventDispatcherGLibmm::start() {
    bool result = false;

    if (mDispatcher) {
        if (mDispatcherConnection.connected() == false) {
            mDispatcherConnection = mDispatcher->connect(sigc::mem_fun(this, &HsmEventDispatcherGLibmm::dispatchPendingEvents));
        }

        result = true;
    }

    return result;
}

void HsmEventDispatcherGLibmm::unregisterAllTimerHandlers() {
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
        // TODO:  mNativeTimerHandlers is not thread-safe
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