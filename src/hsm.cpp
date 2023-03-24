// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/hsm.hpp"

#include "HsmImpl.hpp"

namespace hsmcpp {

HierarchicalStateMachine::HierarchicalStateMachine(const StateID_t initialState)
    : mImpl(new HierarchicalStateMachine::Impl(this, initialState)) {
}

HierarchicalStateMachine::~HierarchicalStateMachine() {
    mImpl->release();
    mImpl->resetParent();
}

void HierarchicalStateMachine::setInitialState(const StateID_t initialState) {
    mImpl->setInitialState(initialState);
}

bool HierarchicalStateMachine::initialize(const std::weak_ptr<IHsmEventDispatcher>& dispatcher) {
    return mImpl->initialize(dispatcher);
}

bool HierarchicalStateMachine::isInitialized() const {
    return mImpl->isInitialized();
}

void HierarchicalStateMachine::release() {
    mImpl->release();
}

void HierarchicalStateMachine::registerFailedTransitionCallback(const HsmTransitionFailedCallback_t& onFailedTransition) {
    mImpl->registerFailedTransitionCallback(onFailedTransition);
}

void HierarchicalStateMachine::registerState(const StateID_t state,
                                             HsmStateChangedCallback_t onStateChanged,
                                             HsmStateEnterCallback_t onEntering,
                                             HsmStateExitCallback_t onExiting) {
    mImpl->registerState(state, onStateChanged, onEntering, onExiting);
}

void HierarchicalStateMachine::registerFinalState(const StateID_t state,
                                                  const EventID_t event,
                                                  HsmStateChangedCallback_t onStateChanged,
                                                  HsmStateEnterCallback_t onEntering,
                                                  HsmStateExitCallback_t onExiting) {
    mImpl->registerFinalState(state, event, onStateChanged, onEntering, onExiting);
}

void HierarchicalStateMachine::registerHistory(const StateID_t parent,
                                               const StateID_t historyState,
                                               const HistoryType type,
                                               const StateID_t defaultTarget,
                                               HsmTransitionCallback_t transitionCallback) {
    mImpl->registerHistory(parent, historyState, type, defaultTarget, transitionCallback);
}

bool HierarchicalStateMachine::registerSubstate(const StateID_t parent, const StateID_t substate) {
    return mImpl->registerSubstate(parent, substate);
}

bool HierarchicalStateMachine::registerSubstateEntryPoint(const StateID_t parent,
                                                          const StateID_t substate,
                                                          const EventID_t onEvent,
                                                          const HsmTransitionConditionCallback_t& conditionCallback,
                                                          const bool expectedConditionValue) {
    return mImpl->registerSubstateEntryPoint(parent, substate, onEvent, conditionCallback, expectedConditionValue);
}

void HierarchicalStateMachine::registerTimer(const TimerID_t timerID, const EventID_t event) {
    mImpl->registerTimer(timerID, event);
}

void HierarchicalStateMachine::registerTransition(const StateID_t from,
                                                  const StateID_t to,
                                                  const EventID_t onEvent,
                                                  HsmTransitionCallback_t transitionCallback,
                                                  HsmTransitionConditionCallback_t conditionCallback,
                                                  const bool expectedConditionValue) {
    mImpl->registerTransition(from, to, onEvent, transitionCallback, conditionCallback, expectedConditionValue);
}

void HierarchicalStateMachine::registerSelfTransition(const StateID_t state,
                                                      const EventID_t onEvent,
                                                      const TransitionType type,
                                                      HsmTransitionCallback_t transitionCallback,
                                                      HsmTransitionConditionCallback_t conditionCallback,
                                                      const bool expectedConditionValue) {
    mImpl->registerSelfTransition(state, onEvent, type, transitionCallback, conditionCallback, expectedConditionValue);
}

StateID_t HierarchicalStateMachine::getLastActiveState() const {
    return mImpl->getLastActiveState();
}

const std::list<StateID_t>& HierarchicalStateMachine::getActiveStates() const {
    return mImpl->getActiveStates();
}

bool HierarchicalStateMachine::isStateActive(const StateID_t state) const {
    return mImpl->isStateActive(state);
}

void HierarchicalStateMachine::transitionWithArgsArray(const EventID_t event, const VariantVector_t& args) {
    return mImpl->transitionWithArgsArray(event, args);
}

bool HierarchicalStateMachine::transitionExWithArgsArray(const EventID_t event,
                                                         const bool clearQueue,
                                                         const bool sync,
                                                         const int timeoutMs,
                                                         const VariantVector_t& args) {
    return mImpl->transitionExWithArgsArray(event, clearQueue, sync, timeoutMs, args);
}

bool HierarchicalStateMachine::transitionInterruptSafe(const EventID_t event) {
    return mImpl->transitionInterruptSafe(event);
}

void HierarchicalStateMachine::startTimer(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot) {
    mImpl->startTimer(timerID, intervalMs, isSingleShot);
}

void HierarchicalStateMachine::restartTimer(const TimerID_t timerID) {
    mImpl->restartTimer(timerID);
}

void HierarchicalStateMachine::stopTimer(const TimerID_t timerID) {
    mImpl->stopTimer(timerID);
}

bool HierarchicalStateMachine::isTimerRunning(const TimerID_t timerID) {
    return mImpl->isTimerRunning(timerID);
}

bool HierarchicalStateMachine::enableHsmDebugging() {
    return mImpl->enableHsmDebugging();
}

bool HierarchicalStateMachine::enableHsmDebugging(const std::string& dumpPath) {
    return mImpl->enableHsmDebugging(dumpPath);
}

void HierarchicalStateMachine::disableHsmDebugging() {
    mImpl->disableHsmDebugging();
}

std::string HierarchicalStateMachine::getStateName(const StateID_t state) const {
    std::string name;

    if (state != INVALID_HSM_STATE_ID) {
        name = std::to_string(state);
    }

    return name;
}

std::string HierarchicalStateMachine::getEventName(const EventID_t event) const {
    std::string name;

    if (event != INVALID_HSM_EVENT_ID) {
        name = std::to_string(event);
    }

    return name;
}

bool HierarchicalStateMachine::isTransitionPossibleImpl(const EventID_t event, const VariantVector_t& args) {
    return mImpl->isTransitionPossible(event, args);
}

bool HierarchicalStateMachine::registerStateActionImpl(const StateID_t state,
                                                       const StateActionTrigger actionTrigger,
                                                       const StateAction action,
                                                       const VariantVector_t& args) {
    return mImpl->registerStateAction(state, actionTrigger, action, args);
}

}  // namespace hsmcpp