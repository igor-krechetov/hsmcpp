// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherSTD.hpp"
#include "hsmcpp/logging.hpp"

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "HsmEventDispatcherSTD"

HsmEventDispatcherSTD::HsmEventDispatcherSTD()
{
}

HsmEventDispatcherSTD::~HsmEventDispatcherSTD()
{
    __TRACE_CALL_DEBUG__();

    unregisterAllEventHandlers();
    stop();
    join();
}

int HsmEventDispatcherSTD::registerEventHandler(std::function<void(void)> handler)
{
    __TRACE_CALL_DEBUG__();
    int id = getNextHandlerID();

    mEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherSTD::unregisterEventHandler(const int handlerId)
{
    std::lock_guard<std::mutex> lck(mHandlersSync);

    __TRACE_CALL_DEBUG_ARGS__("handlerId=%d", handlerId);
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherSTD::emit()
{
    __TRACE_CALL_DEBUG__();
    if (true == mDispatcherThread.joinable())
    {
        std::lock_guard<std::mutex> lck(mEmitSync);

        ++mPendingEmitCount;
        mEmitEvent.notify_one();
    }
}

bool HsmEventDispatcherSTD::start()
{
    __TRACE_CALL_DEBUG__();
    bool result = false;

    if (false == mDispatcherThread.joinable())
    {
        __TRACE_DEBUG__("starting thread...");
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
    __TRACE_CALL_DEBUG__();

    if (true == mDispatcherThread.joinable())
    {
        mStopDispatcher = true;
        mEmitEvent.notify_all();
    }
}

void HsmEventDispatcherSTD::join()
{
    __TRACE_CALL_DEBUG__();

    if (true == mDispatcherThread.joinable())
    {
        mDispatcherThread.join();
    }
}

void HsmEventDispatcherSTD::unregisterAllEventHandlers()
{
    std::lock_guard<std::mutex> lck(mHandlersSync);

    // NOTE: can be called only in destructor after thread was already stopped
    if (false == mDispatcherThread.joinable())
    {
        mEventHandlers.clear();
    }
}

void HsmEventDispatcherSTD::doDispatching()
{
    __TRACE_CALL_DEBUG__();

    while (false == mStopDispatcher)
    {
        while ((mPendingEmitCount > 0) && (mEventHandlers.size() > 0))
        {
            __TRACE_DEBUG__("handle emit event... (%d)", mPendingEmitCount);

            {
                std::lock_guard<std::mutex> lck(mHandlersSync);

                for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
                {
                    if (true == mStopDispatcher)
                    {
                        __TRACE_DEBUG__("stopping...");
                        break;
                    }

                    it->second();
                }
            }

            --mPendingEmitCount;

            if (true == mStopDispatcher)
            {
                __TRACE_DEBUG__("stopping...");
                break;
            }
        }

        if (false == mStopDispatcher)
        {
            std::unique_lock<std::mutex> lck(mEmitSync);

            if (mPendingEmitCount == 0)
            {
                __TRACE_DEBUG__("wait for emit...");
                mEmitEvent.wait(lck, [=](){ return (mPendingEmitCount > 0) || (true == mStopDispatcher); });
                __TRACE_DEBUG__("woke up. pending events=%d", mPendingEmitCount);
            }
        }
    }

    __TRACE_DEBUG__("EXIT");
}