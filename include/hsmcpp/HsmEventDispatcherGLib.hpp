// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERGLIB_HPP
#define HSMCPP_HSMEVENTDISPATCHERGLIB_HPP

#include "HsmEventDispatcherBase.hpp"
#include <glib.h>
#include <condition_variable>

namespace hsmcpp
{

class HsmEventDispatcherGLib: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherGLib();
    explicit HsmEventDispatcherGLib(GMainContext* context);
    virtual ~HsmEventDispatcherGLib();

    virtual void emitEvent(const HandlerID_t handlerID) override;

    virtual bool start() override;

private:
    static gboolean onPipeDataAvailable(GIOChannel* gio, GIOCondition condition, gpointer data);

private:
    GMainContext* mContext = nullptr;
    GIOChannel* mReadChannel = nullptr;
    GSource* mIoSource = nullptr;
    std::mutex mPipeSync;
    int mPipeFD[2] = {-1, -1};
    bool mStopDispatcher = false;
    bool mDispatchingIterationRunning = false;
    std::mutex mDispatchingSync;
    std::condition_variable mDispatchingDoneEvent;
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERGLIB_HPP
