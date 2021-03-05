// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#include "HsmEventDispatcherSTD.hpp"
#include "logging.hpp"

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "HsmEventDispatcherSTD"

HsmEventDispatcherSTD::HsmEventDispatcherSTD()
    : mPendingEmitCount(0)
    , mStopDispatcher(false)
{
}

HsmEventDispatcherSTD::~HsmEventDispatcherSTD()
{
    __TRACE_CALL_DEBUG__();

    if (true == mDispatcherThread.joinable())
    {
        mStopDispatcher = true;
        mEmitEvent.notify_one();
        mDispatcherThread.join();
    }

    unregisterAllEventHandlers();
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
    __TRACE_CALL_DEBUG_ARGS__("handlerId=%d", handlerId);
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        // TODO: not thread-safe
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherSTD::emit()
{
    __TRACE_CALL_DEBUG__();
    if (true == mDispatcherThread.joinable())
    {
        std::unique_lock<std::mutex> lck(mEmitSync);

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

    return result;
}

void HsmEventDispatcherSTD::unregisterAllEventHandlers()
{
    // NOTE: can be called only in destructor after thread was already stopped
    if (false == mDispatcherThread.joinable())
    {
        mEventHandlers.clear();
    }
}

void HsmEventDispatcherSTD::doDispatching()
{
    __TRACE_CALL_DEBUG__();

    while (false == mStopDispatcher.load())
    {
        while (mPendingEmitCount.load() > 0)
        {
            __TRACE_DEBUG__("handle emit event...");
            // TODO: not thread-safe
            for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
            {
                if (true == mStopDispatcher.load())
                {
                    __TRACE_DEBUG__("stopping...");
                    break;
                }

                it->second();
            }

            --mPendingEmitCount;

            if (true == mStopDispatcher.load())
            {
                __TRACE_DEBUG__("stopping...");
                break;
            }
        }

        if (false == mStopDispatcher.load())
        {
            std::unique_lock<std::mutex> lck(mEmitSync);

            if (mPendingEmitCount.load() == 0)
            {
                __TRACE_DEBUG__("wait for emit...");
                mEmitEvent.wait(lck);
                __TRACE_DEBUG__("woke up. pending events=%d", mPendingEmitCount.load());
            }
        }
    }

    __TRACE_DEBUG__("EXIT");
}