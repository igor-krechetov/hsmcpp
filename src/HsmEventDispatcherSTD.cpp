// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherSTD.hpp"
#include "hsmcpp/logging.hpp"

namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherSTD"

HsmEventDispatcherSTD::~HsmEventDispatcherSTD()
{
    __HSM_TRACE_CALL_DEBUG__();

    unregisterAllEventHandlers();
    stop();
    join();
}

void HsmEventDispatcherSTD::emitEvent(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG__();

    if (true == mDispatcherThread.joinable())
    {
        HsmEventDispatcherBase::emitEvent(handlerID);
        mEmitEvent.notify();
    }
}

bool HsmEventDispatcherSTD::start()
{
    __HSM_TRACE_CALL_DEBUG__();
    bool result = false;

    if (false == mDispatcherThread.joinable())
    {
        __HSM_TRACE_DEBUG__("starting thread...");
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
    __HSM_TRACE_CALL_DEBUG__();

    if (true == mDispatcherThread.joinable())
    {
        mStopDispatcher = true;
        mEmitEvent.notify();
    }
}

void HsmEventDispatcherSTD::join()
{
    __HSM_TRACE_CALL_DEBUG__();

    if (true == mDispatcherThread.joinable())
    {
        mDispatcherThread.join();
    }
}

void HsmEventDispatcherSTD::doDispatching()
{
    __HSM_TRACE_CALL_DEBUG__();

    while (false == mStopDispatcher)
    {
        HsmEventDispatcherBase::dispatchPendingEvents();

        if (false == mStopDispatcher)
        {
            UniqueLock lck(mEmitSync);

            if (true == mPendingEvents.empty())
            {
                __HSM_TRACE_DEBUG__("wait for emit...");
                mEmitEvent.wait(lck, [=](){ return (false == mPendingEvents.empty()) || (true == mStopDispatcher); });
                __HSM_TRACE_DEBUG__("woke up. pending events=%lu", mPendingEvents.size());
            }
        }
    }

    __HSM_TRACE_DEBUG__("EXIT");
}

}