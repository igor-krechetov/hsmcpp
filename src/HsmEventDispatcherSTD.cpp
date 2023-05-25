// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherSTD.hpp"

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherSTD"

HsmEventDispatcherSTD::HsmEventDispatcherSTD(const size_t eventsCacheSize)
    // cppcheck-suppress misra-c2012-10.4 ; false-positive. thinks that ':' is arithmetic operation
    : HsmEventDispatcherBase(eventsCacheSize) {
    HSM_TRACE_CALL_DEBUG();
}

HsmEventDispatcherSTD::~HsmEventDispatcherSTD() {
    HSM_TRACE_CALL_DEBUG();

    HsmEventDispatcherSTD::stop();
    join();
}

std::shared_ptr<HsmEventDispatcherSTD> HsmEventDispatcherSTD::create(const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherSTD>(new HsmEventDispatcherSTD(eventsCacheSize),
                                                  &HsmEventDispatcherBase::handleDelete);
}

bool HsmEventDispatcherSTD::deleteSafe() {
    // NOTE: just delete the instance. Calling destructor from any thread is safe
    return true;
}

void HsmEventDispatcherSTD::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();

    if (true == mDispatcherThread.joinable()) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

bool HsmEventDispatcherSTD::start() {
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    if (false == mDispatcherThread.joinable()) {
        HSM_TRACE_DEBUG("starting thread...");
        mStopDispatcher = false;
        mDispatcherThread = std::thread(&HsmEventDispatcherSTD::doDispatching, this);
        result = mDispatcherThread.joinable();
    } else {
        result = (mDispatcherThread.get_id() != std::thread::id());
    }

    return result;
}

void HsmEventDispatcherSTD::stop() {
    HSM_TRACE_CALL_DEBUG();
    UniqueLock lck(mEmitSync);

    HsmEventDispatcherBase::stop();
    unregisterAllEventHandlers();
    notifyDispatcherAboutEvent();
    notifyTimersThread();
}

void HsmEventDispatcherSTD::join() {
    HSM_TRACE_CALL_DEBUG();

    if (true == mDispatcherThread.joinable()) {
        mDispatcherThread.join();
    }

    if (true == mTimersThread.joinable()) {
        mTimersThread.join();
    }
}

void HsmEventDispatcherSTD::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    HSM_TRACE_CALL_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d", SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));
    auto it = mRunningTimers.end();

    // lazy initialization of timers thread
    if (false == mTimersThread.joinable()) {
        mTimersThread = std::thread(&HsmEventDispatcherSTD::handleTimers, this);
    }

    {
        CriticalSection cs(mRunningTimersSync);
        it = mRunningTimers.find(timerID);

        // new timer
        if (mRunningTimers.end() == it) {
            RunningTimerInfo newTimer;

            newTimer.startedAt = std::chrono::steady_clock::now();
            newTimer.elapseAfter = newTimer.startedAt + std::chrono::milliseconds(intervalMs);

            mRunningTimers[timerID] = newTimer;
        } else {
            // restart timer
            it->second.startedAt = std::chrono::steady_clock::now();
            it->second.elapseAfter = it->second.startedAt + std::chrono::milliseconds(intervalMs);
        }
    }

    // wakeup timers thread
    notifyTimersThread();
}

void HsmEventDispatcherSTD::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_ARGS("timerID=%d", SC2INT(timerID));

    {
        CriticalSection cs(mRunningTimersSync);
        mRunningTimers.erase(timerID);
    }

    // wakeup timers thread
    notifyTimersThread();
}

void HsmEventDispatcherSTD::notifyDispatcherAboutEvent() {
    mEmitEvent.notify();
}

void HsmEventDispatcherSTD::doDispatching() {
    HSM_TRACE_CALL_DEBUG();

    while (false == mStopDispatcher) {
        HsmEventDispatcherBase::dispatchPendingEvents();

        if (false == mStopDispatcher) {
            UniqueLock lck(mEmitSync);

            if (true == mPendingEvents.empty()) {
                HSM_TRACE_DEBUG("wait for emit...");
                // NOTE: false-positive. "A function should have a single point of exit at the end" is not vialated because
                //       "return" statement belogs to a lamda function, not doDispatching.
                // cppcheck-suppress misra-c2012-15.5
                mEmitEvent.wait(lck, [=]() {
                    // cppcheck-suppress misra-c2012-15.5 ; false-positive. "return" statement belongs to lambda function
                    return (false == mPendingEvents.empty()) || (false == mEnqueuedEvents.empty()) || (true == mStopDispatcher);
                });
                HSM_TRACE_DEBUG("woke up. pending events=%lu", mPendingEvents.size());
            }
        }
    }

    HSM_TRACE_DEBUG("EXIT");
}

void HsmEventDispatcherSTD::notifyTimersThread() {
    HSM_TRACE_CALL_DEBUG();
    mNotifiedTimersThread = true;
    mTimerEvent.notify();
}

void HsmEventDispatcherSTD::handleTimers() {
    HSM_TRACE_CALL_DEBUG();
    Mutex timerEventSync;
    UniqueLock lck(timerEventSync);

    while (false == mStopDispatcher) {
        mRunningTimersSync.lock();

        if (false == mRunningTimers.empty()) {
            auto itTimeout = mRunningTimers.begin();

            for (auto it = mRunningTimers.begin(); it != mRunningTimers.end(); ++it) {
                if (it->second.elapseAfter < itTimeout->second.elapseAfter) {
                    itTimeout = it;
                }
            }

            TimerID_t waitingTimerId = itTimeout->first;
            const int intervalMs = std::chrono::duration_cast<std::chrono::milliseconds>(itTimeout->second.elapseAfter -
                                                                                         std::chrono::steady_clock::now())
                                       .count();

            mRunningTimersSync.unlock();

            // NOTE: false-positive. "A function should have a single point of exit at the end" is not vialated because
            //       "return" statement belogs to a lamda function, not doDispatching.
            // cppcheck-suppress misra-c2012-15.5
            const bool waitResult = mTimerEvent.wait_for(lck, intervalMs, [&]() { return mNotifiedTimersThread; });

            mNotifiedTimersThread = false;

            if (false == waitResult) {
                // timeout expired
                CriticalSection lckExpired(mRunningTimersSync);

                itTimeout = mRunningTimers.find(waitingTimerId);

                if (itTimeout != mRunningTimers.end()) {
                    if (true == handleTimerEvent(waitingTimerId)) {
                        // restart timer
                        itTimeout->second.startedAt = std::chrono::steady_clock::now();
                        itTimeout->second.elapseAfter = itTimeout->second.startedAt + std::chrono::milliseconds(intervalMs);
                    } else {
                        // single shot timer. remove from queue
                        mRunningTimers.erase(itTimeout);
                    }
                } else {
                    // do nothing
                }
            } else {
                // thread was woken up by start/stop operation. no need to do anything
            }
        } else {
            mRunningTimersSync.unlock();

            // wait for timer events
            mTimerEvent.wait(lck);
        }
    }

    HSM_TRACE_DEBUG("EXIT");
}

}  // namespace hsmcpp