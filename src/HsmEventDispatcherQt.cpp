// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherQt.hpp"
#include "hsmcpp/logging.hpp"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "HsmEventDispatcherQt"

#define QT_EVENT_OFFSET                         (777)

QEvent::Type HsmEventDispatcherQt::mQtEventType = QEvent::None;

HsmEventDispatcherQt::HsmEventDispatcherQt() : QObject(nullptr)
{
}

HsmEventDispatcherQt::~HsmEventDispatcherQt()
{
    __TRACE_CALL_DEBUG__();

    unregisterAllEventHandlers();
}

bool HsmEventDispatcherQt::start()
{
    __TRACE_CALL_DEBUG__();
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

int HsmEventDispatcherQt::registerEventHandler(std::function<void(void)> handler)
{
    __TRACE_CALL_DEBUG__();
    int id = getNextHandlerID();

    mEventHandlers.emplace(id, handler);

    return id;
}

void HsmEventDispatcherQt::unregisterEventHandler(const int handlerId)
{
    std::lock_guard<std::mutex> lck(mHandlersSync);

    __TRACE_CALL_DEBUG_ARGS__("handlerId=%d", handlerId);
    auto it = mEventHandlers.find(handlerId);

    if (it != mEventHandlers.end())
    {
        mEventHandlers.erase(it);
    }
}

void HsmEventDispatcherQt::emitEvent()
{
    __TRACE_CALL_DEBUG__();

    if (QEvent::None != mQtEventType)
    {
        QCoreApplication::postEvent(this, new QEvent(mQtEventType));
    }
}

void HsmEventDispatcherQt::unregisterAllEventHandlers()
{
    __TRACE_CALL_DEBUG__();
    std::lock_guard<std::mutex> lck(mHandlersSync);

    // NOTE: can be called only in destructor after thread was already stopped
    mEventHandlers.clear();
}

bool HsmEventDispatcherQt::event(QEvent* ev)
{
    __TRACE_CALL_DEBUG__();
    bool processed = false;

    if (ev->type() == mQtEventType)
    {
        std::lock_guard<std::mutex> lck(mHandlersSync);

        for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
        {
            it->second();
        }

        processed = true;
    }

    return processed;
}
