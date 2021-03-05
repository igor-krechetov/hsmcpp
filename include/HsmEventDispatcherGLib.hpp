// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for detail

#ifndef __HSMCPP_HSMEVENTDISPATCHERGLIB_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERGLIB_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <glibmm.h>
#include <memory>
#include <map>

class HsmEventDispatcherGLib: public HsmEventDispatcherBase
{
public:
    HsmEventDispatcherGLib();
    HsmEventDispatcherGLib(const Glib::RefPtr<Glib::MainContext>& context);
    virtual ~HsmEventDispatcherGLib();

    virtual int registerEventHandler(std::function<void(void)> handler) override;
    virtual void unregisterEventHandler(const int handlerId) override;
    virtual void emit() override;

    virtual bool start() override;

protected:
    virtual void unregisterAllEventHandlers() override;

private:
    std::shared_ptr<Glib::Dispatcher> mDispatcher;
    std::map<int, sigc::connection> mEventHandlers;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERGLIB_HPP__
