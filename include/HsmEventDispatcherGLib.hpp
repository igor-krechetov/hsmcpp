// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#ifndef __HSMCPP_HSMEVENTDISPATCHERGLIB_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERGLIB_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <glib.h>
#include <map>
#include <mutex>
#include <condition_variable>

class HsmEventDispatcherGLib: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherGLib();
    explicit HsmEventDispatcherGLib(GMainContext* context);
    virtual ~HsmEventDispatcherGLib();

    virtual int registerEventHandler(std::function<void(void)> handler) override;
    virtual void unregisterEventHandler(const int handlerId) override;
    virtual void emit() override;

    virtual bool start() override;

protected:
    void unregisterAllEventHandlers();

private:
    static gboolean onPipeDataAvailable(GIOChannel* gio, GIOCondition condition, gpointer data);

private:
    GMainContext* mContext = nullptr;
    GIOChannel* mReadChannel = nullptr;
    GSource* mIoSource = nullptr;
    std::map<int, std::function<void(void)>> mEventHandlers;
    std::mutex mSyncPipe;
    int mPipeFD[2] = {-1, -1};
    bool mStopDispatcher = false;
    bool mDispatchingIterationRunning = false;
    std::mutex mSyncEventHandlers;
    std::condition_variable mDispatchingDoneEvent;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERGLIB_HPP__
