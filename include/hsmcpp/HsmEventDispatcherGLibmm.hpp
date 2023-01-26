// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP
#define HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP

#include "HsmEventDispatcherBase.hpp"
#include <glibmm.h>
#include <memory>

namespace hsmcpp
{

// NOTE: this implementation is based on Glib::Dispatcher class so it has to follow the same rules.
//       Most important one are:
//        * HsmEventDispatcherGLibmm must be constructred and destroyed in the receiver
//          thread (the thread in whose main loop it will execute its connected slots)
//        * registerEventHandler() must be called from the same thread where dispatcher was created
//       for more details see: https://developer.gnome.org/gtkmm-tutorial/stable/sec-using-glib-dispatcher.html.en
//
//       Not following these rules will result in an occasional SIGSEGV crash (usually when deleting dispatcher instance).
//
//       Unless you really have to it's always better to reuse a single dispatcher instance for multiple HSMs instead of
//       creating/deleting multiple ones(they will anyway handle events sequentially since they use same Glib main loop)

class HsmEventDispatcherGLibmm: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherGLibmm();
    explicit HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context);
    virtual ~HsmEventDispatcherGLibmm();

    virtual bool start() override;

    virtual void emitEvent(const HandlerID_t handlerID) override;

protected:
    void unregisterAllTimerHandlers();

    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;

    void onDispatchEvents();
    bool onTimerEvent(const TimerID_t timerID);

private:
    Glib::RefPtr<Glib::MainContext> mMainContext;
    std::unique_ptr<Glib::Dispatcher> mDispatcher;
    sigc::connection mDispatcherConnection;
    std::map<TimerID_t, sigc::connection> mNativeTimerHandlers;// <timerID, connection>
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP
