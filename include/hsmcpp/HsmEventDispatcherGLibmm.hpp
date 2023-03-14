// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP
#define HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP

#include <glibmm.h>

#include <memory>

#include "HsmEventDispatcherBase.hpp"

namespace hsmcpp {

/**
 * @brief HsmEventDispatcherGLibmm provides dispatcher implementation based on Glibmm library.
 * @details HsmEventDispatcherGLibmm implementation is based on Glib::Dispatcher class so it has to follow the same rules. Most
 * important ones are:
 *      \li HsmEventDispatcherGLibmm must be constructred and destroyed in the receiver thread (the thread in whose main loop it
 * will execute its connected slots)
 *      \li registerEventHandler() must be called from the same thread where dispatcher was created
 * for more details see https://developer.gnome.org/gtkmm-tutorial/stable/sec-using-glib-dispatcher.html.en
 *
 * Not following these rules will result in an occasional SIGSEGV crash (usually when deleting dispatcher instance).
 * Unless you really have to, it's always better to reuse a single dispatcher instance for multiple HSMs instead of
 * creating/deleting multiple ones (they will anyway handle events sequentially since they use same Glib main loop).
 *
 * See @rstref{platforms-dispatcher-glibmm} for details.
 */
class HsmEventDispatcherGLibmm : public HsmEventDispatcherBase {
public:
    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
     * @details Uses default GLib context.
    */
    explicit HsmEventDispatcherGLibmm(const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);

    /**
     * @copydoc HsmEventDispatcherBase::HsmEventDispatcherBase()
     * @param context custom GLib context to use for dispatcher.
    */
    HsmEventDispatcherGLibmm(const Glib::RefPtr<Glib::MainContext>& context,
                             const size_t eventsCacheSize = DISPATCHER_DEFAULT_EVENTS_CACHESIZE);
    /**
     * Destructor.
     */
    virtual ~HsmEventDispatcherGLibmm();

    /**
     * @brief See IHsmEventDispatcher::start()
     * @notthreadsafe{Thread safety is not required by HierarchicalStateMachine::initialize() which uses this API.}
     */
    bool start() override;

    /**
     * @brief See IHsmEventDispatcher::emitEvent()
     * @threadsafe{ }
     */
    void emitEvent(const HandlerID_t handlerID) override;

protected:
    void unregisterAllTimerHandlers();

    /**
     * @brief See HsmEventDispatcherBase::startTimerImpl()
     * @notthreadsafe{Not required by base class}
     */
    void startTimerImpl(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) override;
    /**
     * @brief See HsmEventDispatcherBase::stopTimerImpl()
     * @notthreadsafe{Not required by base class}
     */
    void stopTimerImpl(const TimerID_t timerID) override;

    void notifyDispatcherAboutEvent() override;
    bool onTimerEvent(const TimerID_t timerID);

private:
    Glib::RefPtr<Glib::MainContext> mMainContext;
    std::unique_ptr<Glib::Dispatcher> mDispatcher;
    sigc::connection mDispatcherConnection;
    std::map<TimerID_t, sigc::connection> mNativeTimerHandlers;  // <timerID, connection>
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMEVENTDISPATCHERGLIBMM_HPP
