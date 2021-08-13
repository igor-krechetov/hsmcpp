// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERQT_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERQT_HPP__

#include "HsmEventDispatcherBase.hpp"
#include <map>
#include <mutex>
#include <functional>

#include <QObject>
#include <QEvent>

namespace hsmcpp
{

class HsmEventDispatcherQt: public QObject
                          , public HsmEventDispatcherBase
{
    Q_OBJECT

public:
    HsmEventDispatcherQt();
    virtual ~HsmEventDispatcherQt();

    bool start() override;

    HandlerID_t registerEventHandler(const EventHandlerFunc_t& handler) override;
    void unregisterEventHandler(const HandlerID_t handlerID) override;
    void emitEvent() override;

protected:
    void unregisterAllEventHandlers();
    bool event(QEvent* ev) override;

private:
    static QEvent::Type mQtEventType;

    std::mutex mHandlersSync;
    std::map<HandlerID_t, EventHandlerFunc_t> mEventHandlers;
};

} // namespace hsmcpp

#endif // __HSMCPP_HSMEVENTDISPATCHERQT_HPP__
