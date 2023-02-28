// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_HSMTYPES_HPP
#define HSMCPP_HSMTYPES_HPP

#include <functional>

#include "variant.hpp"

namespace hsmcpp {

using HandlerID_t = int32_t;
using TimerID_t = int32_t;
using EventID_t = int32_t;
using StateID_t = int32_t;

#define HSM_WAIT_INDEFINITELY (0)

#define INVALID_ID (-1000)
#define INVALID_HSM_EVENT_ID static_cast<hsmcpp::EventID_t>(INVALID_ID)
#define INVALID_HSM_STATE_ID static_cast<hsmcpp::StateID_t>(INVALID_ID)
#define INVALID_HSM_TIMER_ID static_cast<hsmcpp::StateID_t>(INVALID_ID)

#define INVALID_HSM_DISPATCHER_HANDLER_ID (0)

using HsmTransitionCallback_t = std::function<void(const VariantVector_t&)>;
using HsmTransitionConditionCallback_t = std::function<bool(const VariantVector_t&)>;
using HsmStateChangedCallback_t = std::function<void(const VariantVector_t&)>;
using HsmStateEnterCallback_t = std::function<bool(const VariantVector_t&)>;
using HsmStateExitCallback_t = std::function<bool(void)>;
using HsmTransitionFailedCallback_t = std::function<void(const EventID_t, const VariantVector_t&)>;

// NOTE: enclosing input expressions in parentheses is not needed (and will not compile)
// cppcheck-suppress misra-c2012-20.7
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