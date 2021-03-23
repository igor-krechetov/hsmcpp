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

class HsmEventDispatcherQt: public QObject
                          , public HsmEventDispatcherBase
{
    Q_OBJECT

public:
    HsmEventDispatcherQt();
    virtual ~HsmEventDispatcherQt();

    bool start() override;

    int registerEventHandler(std::function<void(void)> handler) override;
    void unregisterEventHandler(const int handlerId) override;
    void emitEvent() override;

protected:
    void unregisterAllEventHandlers();
    bool event(QEvent* ev) override;

private:
    static QEvent::Type mQtEventType;

    std::mutex mHandlersSync;
    std::map<int, std::function<void(void)>> mEventHandlers;
};

#endif // __HSMCPP_HSMEVENTDISPATCHERQT_HPP__
