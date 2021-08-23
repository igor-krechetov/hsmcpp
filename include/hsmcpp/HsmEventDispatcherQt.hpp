// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_HSMEVENTDISPATCHERQT_HPP__
#define __HSMCPP_HSMEVENTDISPATCHERQT_HPP__

#include "HsmEventDispatcherBase.hpp"
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

    void emitEvent(const HandlerID_t handlerID) override;

protected:
    bool event(QEvent* ev) override;

private:
    static QEvent::Type mQtEventType;
};

} // namespace hsmcpp

#endif // __HSMCPP_HSMEVENTDISPATCHERQT_HPP__
