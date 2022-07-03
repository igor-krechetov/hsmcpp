// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherQt.hpp"
#include "hsmcpp/logging.hpp"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QVariant>
#include <QThread>

namespace hsmcpp
{

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "HsmEventDispatcherQt"

#define QT_EVENT_OFFSET                         (777)

QEvent::Type HsmEventDispatcherQt::mQtEventType = QEvent::None;

HsmEventDispatcherQt::HsmEventDispatcherQt() : QObject(nullptr)
{
}

HsmEventDispatcherQt::~HsmEventDispatcherQt()
{
    __HSM_TRACE_CALL_DEBUG__();

    unregisterAllTimerHandlers();
    unregisterAllEventHandlers();
}

bool HsmEventDispatcherQt::start()
{
    __HSM_TRACE_CALL_DEBUG__();
    bool result = true;

    if (QEvent::None == mQtEventType)
    {
        int newEvent = QEvent::registerEventType(static_cast<int>(QEvent::User) + QT_EVENT_OFFSET);

        if (newEvent > 0)
        {
            mQtEventType = static_cast<QEvent::Type>(newEvent);
        }
        else
        {
            result = false;
        }
    }

    if (true == result)
    {
        // if dispatcher wasn't created on main thread we need to move it there
        if (QObject::thread() != QCoreApplication::eventDispatcher()->thread())
        {
            QObject::moveToThread(QCoreApplication::eventDispatcher()->thread());
        }
    }

    return result;
}

void HsmEventDispatcherQt::emitEvent(const HandlerID_t handlerID)
{
    __HSM_TRACE_CALL_DEBUG__();

    if (QEvent::None != mQtEventType)
    {
        HsmEventDispatcherBase::emitEvent(handlerID);
        QCoreApplication::postEvent(this, new QEvent(mQtEventType));
    }
}

void HsmEventDispatcherQt::unregisterAllTimerHandlers()
{
    for (auto it = mNativeTimerHandlers.begin(); it != mNativeTimerHandlers.end(); ++it)
    {
        it->second->deleteLater();
    }
    mNativeTimerHandlers.clear();
}

void HsmEventDispatcherQt::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d, intervalMs=%d, isSingleShot=%d",
                                  SC2INT(timerID), intervalMs, BOOL2INT(isSingleShot));
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() == it)
    {
        auto funcCreateTimer = [&]() {
            QTimer* newTimer = new QTimer(this);

            newTimer->setProperty("hsmid", QVariant(timerID));
            connect(newTimer, SIGNAL(timeout()), this, SLOT(onTimerEvent()));
            newTimer->setSingleShot(isSingleShot);
            newTimer->start(intervalMs);
            mNativeTimerHandlers.emplace(timerID, newTimer);
        };

        // NOTE: need to make sure that QTimer is started only from Qt main thread
        QThread* callerThread = QThread::currentThread();

        if (qApp->thread() != callerThread)
        {
            QTimer* mainthreadTimer = new QTimer();

            mainthreadTimer->moveToThread(qApp->thread());
            mainthreadTimer->setSingleShot(true);

            QObject::connect(mainthreadTimer, &QTimer::timeout, [=]()
            {
                funcCreateTimer();
                mainthreadTimer->deleteLater();
            });
            QMetaObject::invokeMethod(mainthreadTimer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
        }
        else
        {
            funcCreateTimer();
        }
    }
    else
    {
        __HSM_TRACE_ERROR__("timer with id=%d already exists", timerID);
    }
}

void HsmEventDispatcherQt::stopTimerImpl(const TimerID_t timerID)
{
    __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() != it)
    {
        it->second->deleteLater();
        mNativeTimerHandlers.erase(it);
    }
}

void HsmEventDispatcherQt::onTimerEvent()
{
    QObject* ptrTimer = qobject_cast<QTimer*>(QObject::sender());

    if (nullptr != ptrTimer)
    {
        const TimerID_t timerID = ptrTimer->property("hsmid").toInt();

        __HSM_TRACE_CALL_DEBUG_ARGS__("timerID=%d", SC2INT(timerID));
        const bool restartTimer = handleTimerEvent(timerID);

        if (false == restartTimer)
        {
            // TODO:  mNativeTimerHandlers is not thread-safe
            auto itTimer = mNativeTimerHandlers.find(timerID);

            if (mNativeTimerHandlers.end() != itTimer)
            {
                itTimer->second->deleteLater();
                mNativeTimerHandlers.erase(itTimer);
            }
            else
            {
                __HSM_TRACE_ERROR__("unexpected error. timer not found");
            }
        }
    }
}

bool HsmEventDispatcherQt::event(QEvent* ev)
{
    __HSM_TRACE_CALL_DEBUG__();
    bool processed = false;

    if (ev->type() == mQtEventType)
    {
        HsmEventDispatcherBase::dispatchPendingEvents();

        processed = true;
    }

    return processed;
}

} // namespace hsmcpp