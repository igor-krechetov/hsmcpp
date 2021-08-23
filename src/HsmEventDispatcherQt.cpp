// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherQt.hpp"
#include "hsmcpp/logging.hpp"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>

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