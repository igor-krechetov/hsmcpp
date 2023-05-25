// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMTYPES_HPP
#define HSMCPP_HSMTYPES_HPP

#include <functional>

#include "variant.hpp"

namespace hsmcpp {

using HandlerID_t = int32_t;  ///< Handler ID returned by dispatchers for events, timers and enqueued events handlers
/** Type representing timer ID. Used to identify timer when calling timer related API of HierarchicalStateMachine class. */
using TimerID_t = int32_t;
using EventID_t = int32_t;  ///< Type representing HSM event ID. Used when working with HierarchicalStateMachine class.
using StateID_t = int32_t;  ///< Type representing HSM state ID. Used when working with HierarchicalStateMachine class.

/** This macro can be used to indicate to sync API of HierarchicalStateMachine to wait indefinitely for an operation to finish. */
constexpr int HSM_WAIT_INDEFINITELY = 0;

/** Generic value for an invalid ID. Not supposed to be used directly. */
constexpr int INVALID_ID = -1000;
/** Invalid ID for HSM event. Used in relation with EventID_t type. */
constexpr hsmcpp::EventID_t INVALID_HSM_EVENT_ID = static_cast<hsmcpp::EventID_t>(INVALID_ID);
/** Invalid ID for HSM state. Used in relation with StateID_t type. */
constexpr hsmcpp::StateID_t INVALID_HSM_STATE_ID = static_cast<hsmcpp::StateID_t>(INVALID_ID);
/** Invalid ID for HSM timer. Used in relation with TimerID_t type. */
constexpr hsmcpp::TimerID_t INVALID_HSM_TIMER_ID = static_cast<hsmcpp::TimerID_t>(INVALID_ID);

/** Invalid dispatcher handler ID. Used in relation with HandlerID_t type. */
constexpr hsmcpp::HandlerID_t INVALID_HSM_DISPATCHER_HANDLER_ID = 0;

/**
 * Function type for HierarchicalStateMachine transition callbacks.
 *
 * @param VariantVector_t \c args value provided in HierarchicalStateMachine::transition() or similar API
 */
using HsmTransitionCallback_t = std::function<void(const VariantVector_t&)>;
/**
 * Function type for HierarchicalStateMachine condition callbacks.
 *
 * @param VariantVector_t \c args value provided in HierarchicalStateMachine::transition() or similar API
 */
using HsmTransitionConditionCallback_t = std::function<bool(const VariantVector_t&)>;
/**
 * Function type for HierarchicalStateMachine state changed callbacks.
 *
 * @param VariantVector_t \c args value provided in HierarchicalStateMachine::transition() or similar API
 */
using HsmStateChangedCallback_t = std::function<void(const VariantVector_t&)>;
/**
 * Function type for HierarchicalStateMachine state entering callbacks.
 *
 * @param VariantVector_t \c args value provided in HierarchicalStateMachine::transition() or similar API
 *
 * @return Callback should return TRUE to allow current transition. Returning FALSE will cause ongoing transition to be canceled.
 */
using HsmStateEnterCallback_t = std::function<bool(const VariantVector_t&)>;
/**
 * Function type for HierarchicalStateMachine state exiting callbacks.
 * @return Callback should return TRUE to allow current transition. Returning FALSE will cause ongoing transition to be canceled.
 */
using HsmStateExitCallback_t = std::function<bool(void)>;
/**
 * Function type for HierarchicalStateMachine failed transition callbacks. Callback is called whenever HSM failed to process new
 * event (due to no registered transition, failed conditions or transition being canceled by a enter/exit callback).
 *
 * @param std::list<StateID_t> list of active states which didn't have a matching transition
 * @param EventID_t id of the event which was not processed
 * @param VariantVector_t \c args value provided in HierarchicalStateMachine::transition() or similar API
 */
using HsmTransitionFailedCallback_t = std::function<void(const std::list<StateID_t>&, const EventID_t, const VariantVector_t&)>;

// cppcheck-suppress misra-c2012-20.7 ; enclosing input expressions in parentheses is not needed (and will not compile)
#define HsmTransitionCallbackPtr_t(_class, _func) void (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmTransitionConditionCallbackPtr_t(_class, _func) bool (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmStateChangedCallbackPtr_t(_class, _func) void (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmStateEnterCallbackPtr_t(_class, _func) bool (_class::*_func)(const VariantVector_t&)
// cppcheck-suppress misra-c2012-20.7
#define HsmStateExitCallbackPtr_t(_class, _func) bool (_class::*_func)()
// cppcheck-suppress misra-c2012-20.7
#define HsmTransitionFailedCallbackPtr_t(_class, _func) void (_class::*_func)(const std::list<StateID_t>&, const EventID_t, const VariantVector_t&)

/**
 * @enum HistoryType
 * @brief Defines the type of history state.
 * @details The history state can be shallow or deep (see @rstref{features-history} for details).
 */
enum class HistoryType {
    SHALLOW,  ///< remember only the immediate substate of the parent state
    DEEP      ///< remember the last active state of the substate hierarchy
};

/**
 * @enum TransitionType
 * Defines the type of a self transition (see @rstref{features-transitions-selftransitions} for
 * details).
 */
enum class TransitionType {
    INTERNAL_TRANSITION,  ///< do not cause a state change during self transition
    EXTERNAL_TRANSITION   ///< exit current state during self transition
};

/**
 * @enum StateActionTrigger
 * Defines the trigger for a state action (see @rstref{features-states-actions} for details).
 */
enum class StateActionTrigger {
    ON_STATE_ENTRY,  ///< trigger action on state entry
    ON_STATE_EXIT    ///< trigger action on state exit
};

/**
 * @enum StateAction
 * @brief Defines the type of state action (see @rstref{features-states-actions} for details).
 *
 * @details The state action can start, stop, or restart a timer, or cause a transition to another state.
 * Depending on the action type, you need to provide specific arguments when calling registerStateAction()
 */
enum class StateAction {
    START_TIMER,    ///< **Arguments**: TimerID_t timerID, int32_t intervalMs, bool singleshot
    STOP_TIMER,     ///< **Arguments**: TimerID_t timerID
    RESTART_TIMER,  ///< **Arguments**: TimerID_t timerID
    TRANSITION,     ///< **Arguments**: EventID_t eventID
};

}  // namespace hsmcpp

#endif  // HSMCPP_HSMTYPES_HPP