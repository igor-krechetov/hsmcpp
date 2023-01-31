// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherSTD.hpp"
#include "hsmcpp/logging.hpp"

namespace hsmcpp
{

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS                         "HsmEventDispatcherSTD"

HsmEventDispatcherSTD::~HsmEventDispatcherSTD()
{
    HSM_TRACE_CALL_DEBUG();

    unregisterAllEventHandlers();
    stop();
    join();
}

void HsmEventDispatcherSTD::emitEvent(const HandlerID_t handlerID)
{
    HSM_TRACE_CALL_DEBUG();

    if (true == mDispatcherThread.joinable())
    {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

bool HsmEventDispatcherSTD::start()
{
    HSM_TRACE_CALL_DEBUG();
    bool result = false;

    if (false == mDispatcherThread.joinable())
    {
        HSM_TRACE_DEBUG("starting thread...");
        mStopDispatcher = false;
        mDispatcherThread = std::thread(&HsmEventDispatcherSTD::doDispatching, this);
        result = mDispatcherThread.joinable();
    }
    else
    {
        result = (mDispatcherThread.get_id() != std::thread::id());
    }

    return result;
}

void HsmEventDispatcherSTD::stop()
{
    HSM_TRACE_CALL_DEBUG();

    if (true == mDispatcherThread.joinable())
    {
        mStopDispatcher = true;
        notifyDispatcherAboutEvent();
    }
}

void HsmEventDispatcherSTD::join()
{
    HSM_TRACE_CALL_DEBUG();

    if (true == mDispatcherThread.joinable())
    {
        mDispatcherThread.join();
    }
}

void HsmEventDispatcherSTD::notifyDispatcherAboutEvent() {
    mEmitEvent.notify();
}

void HsmEventDispatcherSTD::doDispatching()
{
    HSM_TRACE_CALL_DEBUG();

    while (false == mStopDispatcher)
    {
        HsmEventDispatcherBase::dispatchPendingEvents();

        if (false == mStopDispatcher)
        {
            UniqueLock lck(mEmitSync);

            if (true == mPendingEvents.empty())
            {
                HSM_TRACE_DEBUG("wait for emit...");
                // NOTE: false-positive. "A function should have a single point of exit at the end" is not vialated because
                //       "return" statement belogs to a lamda function, not doDispatching.
                // cppcheck-suppress misra-c2012-15.5
                mEmitEvent.wait(lck, [=](){ return (false == mPendingEvents.empty()) || (true == mStopDispatcher); });
                HSM_TRACE_DEBUG("woke up. pending events=%lu", mPendingEvents.size());
            }
        }
    }

    HSM_TRACE_DEBUG("EXIT");
}

}