// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/HsmEventDispatcherQt.hpp"

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QThread>
#include <QVariant>

#include "hsmcpp/logging.hpp"
#include "hsmcpp/os/CriticalSection.hpp"

namespace hsmcpp {

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "HsmEventDispatcherQt"

#define QT_DISPATCH_EVENT (777)

QEvent::Type HsmEventDispatcherQt::mQtDispatchEventType = QEvent::None;

HsmEventDispatcherQt::HsmEventDispatcherQt(const size_t eventsCacheSize)
    : HsmEventDispatcherBase(eventsCacheSize)
    , QObject(nullptr) {
}

HsmEventDispatcherQt::~HsmEventDispatcherQt() {
    HSM_TRACE_CALL_DEBUG();

    HsmEventDispatcherQt::stop();
}

std::shared_ptr<HsmEventDispatcherQt> HsmEventDispatcherQt::create(const size_t eventsCacheSize) {
    return std::shared_ptr<HsmEventDispatcherQt>(new HsmEventDispatcherQt(eventsCacheSize), &HsmEventDispatcherBase::handleDelete);
}

bool HsmEventDispatcherQt::deleteSafe() {
    QObject::deleteLater();

    return false;
}

bool HsmEventDispatcherQt::start() {
    HSM_TRACE_CALL_DEBUG();
    bool result = true;

    if (QEvent::None == mQtDispatchEventType) {
        int newDispatchEvent = QEvent::registerEventType(static_cast<int>(QEvent::User) + QT_DISPATCH_EVENT);

        if (newDispatchEvent > 0) {
            mQtDispatchEventType = static_cast<QEvent::Type>(newDispatchEvent);
        } else {
            result = false;
        }
    }

    if (true == result) {
        // if dispatcher wasn't created on main thread we need to move it there
        if (QObject::thread() != QCoreApplication::eventDispatcher()->thread()) {
            QObject::moveToThread(QCoreApplication::eventDispatcher()->thread());
        }
    }

    return result;
}

void HsmEventDispatcherQt::stop() {
    HSM_TRACE_CALL_DEBUG();

    HsmEventDispatcherBase::stop();
    unregisterAllTimerHandlers();
    unregisterAllEventHandlers();
}

void HsmEventDispatcherQt::emitEvent(const HandlerID_t handlerID) {
    HSM_TRACE_CALL_DEBUG();

    if (QEvent::None != mQtDispatchEventType) {
        HsmEventDispatcherBase::emitEvent(handlerID);
    }
}

void HsmEventDispatcherQt::unregisterAllTimerHandlers() {
    CriticalSection cs(mRunningTimersSync);

    for (auto it = mNativeTimerHandlers.begin(); it != mNativeTimerHandlers.end(); ++it) {
        it->second->deleteLater();
    }

    mNativeTimerHandlers.clear();
}

void HsmEventDispatcherQt::startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d, intervalMs=%d, isSingleShot=%d",
                              SC2INT(timerID),
                              intervalMs,
                              BOOL2INT(isSingleShot));
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() == it) {
        auto funcCreateTimer = [&]() {
            QTimer* newTimer = new QTimer(this);

            newTimer->setProperty("hsmid", QVariant(timerID));
            connect(newTimer, SIGNAL(timeout()), this, SLOT(onTimerEvent()));
            newTimer->setSingleShot(isSingleShot);
            newTimer->start(intervalMs);

            CriticalSection cs(mRunningTimersSync);
            mNativeTimerHandlers.emplace(timerID, newTimer);
        };

        // NOTE: need to make sure that QTimer is started only from Qt main thread
        QThread* callerThread = QThread::currentThread();

        if (qApp->thread() != callerThread) {
            QTimer* mainthreadTimer = new QTimer();

            mainthreadTimer->moveToThread(qApp->thread());
            mainthreadTimer->setSingleShot(true);

            QObject::connect(mainthreadTimer, &QTimer::timeout, [=]() {
                funcCreateTimer();
                mainthreadTimer->deleteLater();
            });
            QMetaObject::invokeMethod(mainthreadTimer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
        } else {
            funcCreateTimer();
        }
    } else {
        HSM_TRACE_ERROR("timer with id=%d already exists", timerID);
    }
}

void HsmEventDispatcherQt::stopTimerImpl(const TimerID_t timerID) {
    HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
    CriticalSection cs(mRunningTimersSync);
    auto it = mNativeTimerHandlers.find(timerID);

    if (mNativeTimerHandlers.end() != it) {
        it->second->deleteLater();
        mNativeTimerHandlers.erase(it);
    }
}

void HsmEventDispatcherQt::onTimerEvent() {
    QObject* ptrTimer = qobject_cast<QTimer*>(QObject::sender());

    if (nullptr != ptrTimer) {
        const TimerID_t timerID = ptrTimer->property("hsmid").toInt();

        HSM_TRACE_CALL_DEBUG_ARGS("timerID=%d", SC2INT(timerID));
        const bool restartTimer = handleTimerEvent(timerID);

        if (false == restartTimer) {
            CriticalSection cs(mRunningTimersSync);
            auto itTimer = mNativeTimerHandlers.find(timerID);

            if (mNativeTimerHandlers.end() != itTimer) {
                itTimer->second->deleteLater();
                mNativeTimerHandlers.erase(itTimer);
            } else {
                HSM_TRACE_ERROR("unexpected error. timer not found");
            }
        }
    }
}

void HsmEventDispatcherQt::notifyDispatcherAboutEvent() {
    QCoreApplication::postEvent(this, new QEvent(mQtDispatchEventType));
}

void HsmEventDispatcherQt::customEvent(QEvent* ev) {
    HSM_TRACE_CALL_DEBUG();

    if ((false == mStopDispatcher) && (ev->type() == mQtDispatchEventType)) {
        HsmEventDispatcherBase::dispatchPendingEvents();
    }
}

}  // namespace hsmcpp