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

/**
 * This macro can be used to indicate to sync API of HierarchicalStateMachine to wait indefinitely for an operation to finish.
 */
#define HSM_WAIT_INDEFINITELY (0)

/** Generic value for an invalid ID. Not supposed to be used directly. */
#define INVALID_ID (-1000)
/** Invalid ID for HSM event. Used in relation with EventID_t type. */
#define INVALID_HSM_EVENT_ID static_cast<hsmcpp::EventID_t>(INVALID_ID)
/** Invalid ID for HSM state. Used in relation with StateID_t type. */
#define INVALID_HSM_STATE_ID static_cast<hsmcpp::StateID_t>(INVALID_ID)
/** Invalid ID for HSM timer. Used in relation with TimerID_t type. */
#define INVALID_HSM_TIMER_ID static_cast<hsmcpp::TimerID_t>(INVALID_ID)

/** Invalid dispatcher handler ID. Used in relation with HandlerID_t type. */
#define INVALID_HSM_DISPATCHER_HANDLER_ID (static_cast<HandlerID_t>(0))

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
 * @param EventID_t id of the event which was not processed
 * @param VariantVector_t \c args value provided in HierarchicalStateMachine::transition() or similar API
 */
using HsmTransitionFailedCallback_t = std::function<void(const EventID_t, const VariantVector_t&)>;

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
#define HsmTransitionFailedCallbackPtr_t(_class, _func) void (_class::*_func)(const EventID_t, const VariantVector_t&)

}  // namespace hsmcpp

#endif  // HSMCPP_HSMTYPES_HPP