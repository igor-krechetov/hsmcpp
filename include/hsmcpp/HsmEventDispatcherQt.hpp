// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERQT_HPP
#define HSMCPP_HSMEVENTDISPATCHERQT_HPP

#include "HsmEventDispatcherBase.hpp"
#include <QObject>
#include <QEvent>
#include <QTimer>

namespace hsmcpp
{

class HsmEventDispatcherQt: public QObject
                          , public HsmEventDispatcherBase
{
    Q_OBJECT

public:
    // NOTE: false positive. setting default parameter value is not parameter modification
    // cppcheck-suppress misra-c2012-17.8
    explicit HsmEventDispatcherQt(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);
    virtual ~HsmEventDispatcherQt();

    bool start() override;

    void emitEvent(const HandlerID_t handlerID) override;

// Timers
protected:
    void unregisterAllTimerHandlers();

    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;

private slots:
    void onTimerEvent();

protected:
    void notifyDispatcherAboutEvent() override;
    bool event(QEvent* ev) override;

private:
    static QEvent::Type mQtEventType;
    std::map<TimerID_t, QTimer*> mNativeTimerHandlers;// <timerID, QTimer>
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERQT_HPP
