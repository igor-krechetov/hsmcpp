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

/**
 * @brief HsmEventDispatcherQt provides dispatcher implementation based on Qt framework.
 * @details See @rstref{platforms-dispatcher-qt} for details.
 */
class HsmEventDispatcherQt: public QObject
                          , public HsmEventDispatcherBase
{
    Q_OBJECT

public:
    /**
     * @brief Create dispatcher instance.
     * @param eventsCacheSize size of the queue preallocated for delayed events
     * @return New dispatcher instance.
     *
     * @threadsafe{Instance can be safely created and destroyed from any thread.}
     */
    // cppcheck-suppress misra-c2012-17.8 ; false positive. setting default parameter value is not parameter modification
    static std::shared_ptr<HsmEventDispatcherQt> create(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

    /**
     * @brief See IHsmEventDispatcher::start()
     * @notthreadsafe{Thread safety is not required by HierarchicalStateMachine::initialize() which uses this API.}
     */
    bool start() override;

    /**
     * @copydoc IHsmEventDispatcher::stop()
     * @notthreadsafe{TODO: Current timers implementation is not thread-safe}
    */
    void stop() override;

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{ }
     */
    void emitEvent(const HandlerID_t handlerID) override;

// Timers
protected:
    void unregisterAllTimerHandlers();

    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    void stopTimerImpl(const TimerID_t timerID) override;

private slots:
    void onTimerEvent();

protected:
    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
     * @details Uses default GLib content.
    */
    explicit HsmEventDispatcherQt(const size_t eventsCacheSize);

    /**
     * @brief Destructor.
     */
    virtual ~HsmEventDispatcherQt();

    /**
     * @copydoc HsmEventDispatcherBase::deleteSafe()
     */
    bool deleteSafe() override;
    void notifyDispatcherAboutEvent() override;
    void customEvent(QEvent* ev) override;

private:
    static QEvent::Type mQtDispatchEventType;
    std::map<TimerID_t, QTimer*> mNativeTimerHandlers; // protected by mRunningTimersSync
};

} // namespace hsmcpp

#endif // HSMCPP_HSMEVENTDISPATCHERQT_HPP
