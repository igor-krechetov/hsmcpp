// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_SRC_HSMIMPLTYPES_HPP
#define HSMCPP_SRC_HSMIMPLTYPES_HPP

#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "hsmcpp/HsmTypes.hpp"
#include "hsmcpp/os/ConditionVariable.hpp"
#include "hsmcpp/os/Mutex.hpp"
#include "hsmcpp/variant.hpp"

namespace hsmcpp {
enum class HsmLogAction {
    IDLE,
    TRANSITION,
    TRANSITION_ENTRYPOINT,
    CALLBACK_EXIT,
    CALLBACK_ENTER,
    CALLBACK_STATE,
    ON_ENTER_ACTIONS,
    ON_EXIT_ACTIONS,
};

enum class HsmEventStatus { PENDING, DONE_OK, DONE_FAILED, CANCELED };

enum class TransitionBehavior { REGULAR, ENTRYPOINT, FORCED };

struct StateCallbacks {
    HsmStateChangedCallback_t onStateChanged = nullptr;
    HsmStateEnterCallback_t onEntering = nullptr;
    HsmStateExitCallback_t onExiting = nullptr;
};

struct StateEntryPoint {
    StateID_t state = INVALID_HSM_STATE_ID;
    EventID_t onEvent = INVALID_HSM_EVENT_ID;
    HsmTransitionConditionCallback_t checkCondition = nullptr;
    bool expectedConditionValue = true;
};

struct TransitionInfo {
    StateID_t fromState = INVALID_HSM_STATE_ID;
    StateID_t destinationState = INVALID_HSM_STATE_ID;
    TransitionType transitionType = TransitionType::EXTERNAL_TRANSITION;
    HsmTransitionCallback_t onTransition = nullptr;
    HsmTransitionConditionCallback_t checkCondition = nullptr;
    bool expectedConditionValue = true;

    TransitionInfo() = default;

    TransitionInfo(const StateID_t from,
                   const StateID_t to,
                   const TransitionType type,
                   HsmTransitionCallback_t cbTransition,
                   HsmTransitionConditionCallback_t cbCondition);

    TransitionInfo(const StateID_t from,
                   const StateID_t to,
                   const TransitionType type,
                   HsmTransitionCallback_t cbTransition,
                   HsmTransitionConditionCallback_t cbCondition,
                   const bool conditionValue);
};

struct PendingEventInfo {
    TransitionBehavior transitionType = TransitionBehavior::REGULAR;
    EventID_t type = INVALID_HSM_EVENT_ID;
    VariantVector_t args;
    std::shared_ptr<Mutex> cvLock;
    std::shared_ptr<ConditionVariable> syncProcessed;
    std::shared_ptr<HsmEventStatus> transitionStatus;
    std::shared_ptr<std::list<TransitionInfo>> forcedTransitionsInfo;
    bool ignoreEntryPoints = false;

    ~PendingEventInfo();
    void initLock();
    void releaseLock();
    bool isSync() const;
    void wait(const int timeoutMs = HSM_WAIT_INDEFINITELY);
    void unlock(const HsmEventStatus status);
};

struct HistoryInfo {
    HistoryType type = HistoryType::SHALLOW;
    StateID_t defaultTarget = INVALID_HSM_STATE_ID;
    HsmTransitionCallback_t defaultTargetTransitionCallback = nullptr;
    std::list<StateID_t> previousActiveStates;

    HistoryInfo(const HistoryType newType, const StateID_t newDefaultTarget, HsmTransitionCallback_t newTransitionCallback);
};

struct StateActionInfo {
    StateAction action;
    VariantVector_t actionArgs;
};
}  // namespace hsmcpp

#endif  // HSMCPP_SRC_HSMIMPLTYPES_HPP
