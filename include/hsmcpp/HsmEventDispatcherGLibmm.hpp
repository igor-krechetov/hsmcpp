// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <glibmm.h>
#include <memory>
#include <map>

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
//       creating/deliting multiple ones(they will anyway handle events sequentially since they use same Glib main loop)

class HsmEventDispatcherGLibmm: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherGLibmm();
    explicit HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context);
    virtual ~HsmEventDispatcherGLibmm();

    virtual int registerEventHandler(std::function<void(void)> handler) override;
    virtual void unregisterEventHandler(const int handlerId) override;
    virtual void emit() override;

    virtual bool start() override;
    virtual void stop() override;

protected:
    void unregisterAllEventHandlers();

private:
    std::unique_ptr<Glib::Dispatcher> mDispatcher;
    std::map<int, sigc::connection> mEventHandlers;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP__
