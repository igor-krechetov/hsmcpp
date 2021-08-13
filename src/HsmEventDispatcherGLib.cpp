// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherGLib.hpp"
#include "hsmcpp/logging.hpp"
#include <unistd.h>
namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherGLib"

HsmEventDispatcherGLib::HsmEventDispatcherGLib()
{
}

HsmEventDispatcherGLib::HsmEventDispatcherGLib(GMainContext* context)
    : mContext(context)
{
}

HsmEventDispatcherGLib::~HsmEventDispatcherGLib()
{
    __HSM_TRACE_CALL__();
    std::unique_lock<std::mutex> lck(mSyncEventHandlers);

    mStopDispatcher = true;
    unregisterAllEventHandlers();

    if (true == mDispatchingIterationRunning)
    {
        mDispatchingDoneEvent.wait(lck);
    }

    g_source_destroy(mIoSource);
    g_source_unref(mIoSource);
    mIoSource = nullptr;
    g_io_channel_unref(mReadChannel);
    mReadChannel = nullptr;
    close(mPipeFD[0]);
    close(mPipeFD[1]);
    mPipeFD[0] = -1;
    mPipeFD[1] = -1;
}

HandlerID_t HsmEventDispatcherGLib::registerEventHandler(const EventHandlerFunc_t& handler)
{
    __HSM_TRACE_CALL__();
    HandlerID_t id = getNextHandlerID();

    mEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherGLib::unregisterEventHandler(const HandlerID_t handlerId)
{
    __HSM_TRACE_CALL__();
    std::lock_guard<std::mutex> lck(mSyncEventHandlers);
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherGLib::emitEvent()
{
    __HSM_TRACE_CALL__();
    if (mPipeFD[1] > 0)
    {
        std::lock_guard<std::mutex> lck(mSyncPipe);
        char dummy = 1;

        // we just need to trigger the callback. there is no need to pass any real data there
        // TODO: should we check for result?
        write(mPipeFD[1], &dummy, sizeof(dummy));
    }
}

bool HsmEventDispatcherGLib::start()
{
    __HSM_TRACE_CALL_DEBUG__();
    bool result = false;

    // check if dispatcher was already started
    if ((-1) == mPipeFD[0])
    {
        int rc = pipe(mPipeFD);

        if (0 == rc)
        {
            mReadChannel = g_io_channel_unix_new(mPipeFD[0]);

            if (nullptr != mReadChannel)
            {
                mIoSource = g_io_create_watch(mReadChannel, static_cast<GIOCondition>(G_IO_IN | G_IO_HUP));

                if (nullptr != mIoSource)
                {
                    g_source_set_callback(mIoSource, reinterpret_cast<GSourceFunc>(onPipeDataAvailable), this, nullptr);
                    g_source_attach(mIoSource, mContext);
                    result = true;
                }
                else
                {
                    __HSM_TRACE_ERROR__("failed to create io source");
                }
            }
            else
            {
                __HSM_TRACE_ERROR__("failed to create io channel");
            }
        }
        else
        {
            __HSM_TRACE_ERROR__("failed to create pipe (errno=%d)", errno);
        }

        if (false == result)
        {
            if (nullptr != mReadChannel)
            {
                g_io_channel_unref(mReadChannel);
                mReadChannel = nullptr;
            }

            if ((-1) == mPipeFD[0])
            {
                close(mPipeFD[0]);
                close(mPipeFD[1]);
                mPipeFD[0] = -1;
                mPipeFD[1] = -1;
            }
        }
    }
    else
    {
        result = true;
    }

    return result;
}

void HsmEventDispatcherGLib::unregisterAllEventHandlers()
{
    mEventHandlers.clear();
}

gboolean HsmEventDispatcherGLib::onPipeDataAvailable(GIOChannel* gio, GIOCondition condition, gpointer data)
{
    __HSM_TRACE_CALL__();
    gboolean continueDispatching = TRUE;
    HsmEventDispatcherGLib* pThis = static_cast<HsmEventDispatcherGLib*>(data);

    if (false == pThis->mStopDispatcher)
    {
        pThis->mDispatchingIterationRunning = true;

        std::lock_guard<std::mutex> lck(pThis->mSyncEventHandlers);
        __HSM_TRACE__("condition=%d, G_IO_HUP=%s, G_IO_IN=%s", static_cast<int>(condition), BOOL2STR(condition & G_IO_HUP), BOOL2STR(condition & G_IO_IN));

        if (!(condition & G_IO_HUP))
        {
            char dummy;
            int bytes = read(pThis->mPipeFD[0], &dummy, sizeof(dummy));

            if (1 == bytes)
            {
                for (auto it = pThis->mEventHandlers.begin(); (it != pThis->mEventHandlers.end()) && (false == pThis->mStopDispatcher); ++it)
                {
                    it->second();
                }
            }
        }

        if (true == pThis->mStopDispatcher)
        {
            continueDispatching = FALSE;
            pThis->mDispatchingDoneEvent.notify_one();
        }

        pThis->mDispatchingIterationRunning = false;
    }
    else
    {
        continueDispatching = FALSE;
    }

    return continueDispatching;
}

} // namespace hsmcpp