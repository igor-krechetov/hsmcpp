// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP
#define HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP

#include "BaseAsyncHsm.hpp"
#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "TrafficLightHsm"

namespace TrafficLightState {
const hsmcpp::StateID_t OFF = 0;
const hsmcpp::StateID_t STARTING = 1;

const hsmcpp::StateID_t OPERABLE = 2;

const hsmcpp::StateID_t RED = 3;
const hsmcpp::StateID_t YELLOW = 4;
const hsmcpp::StateID_t GREEN = 5;
}  // namespace TrafficLightState

namespace TrafficLightEvent {
const hsmcpp::EventID_t TURN_ON = 0;
const hsmcpp::EventID_t TURN_OFF = 1;
const hsmcpp::EventID_t NEXT_STATE = 2;
}  // namespace TrafficLightEvent

class TrafficLightHsm : public testing::Test, public BaseAsyncHsm, public HierarchicalStateMachine {
public:
    TrafficLightHsm();
    virtual ~TrafficLightHsm();

    void setupDefault();

    DEF_TRANSITION_IMPL(NextState);
    DEF_TRANSITION_IMPL(TurnOff);

    DEF_STATE_ACTION_IMPL(Off)
    DEF_STATE_ACTION_IMPL(Starting)
    DEF_STATE_ACTION_IMPL(Red)
    DEF_STATE_ACTION_IMPL(Yellow)
    DEF_STATE_ACTION_IMPL(Green)

    DEF_STATE_ACTION_IMPL(Dummy)

    DEF_EXIT_ACTION_IMPL(ExitCancel, false)
    DEF_EXIT_ACTION_IMPL(Exit, true)

    DEF_ENTER_ACTION_IMPL(EnterCancel, false)
    DEF_ENTER_ACTION_IMPL(Enter, true)

    bool checkConditionOff2Off(const VariantVector_t& args);
    bool checkConditionOff2On(const VariantVector_t& args);

    void onTransitionFailed(const std::list<hsmcpp::StateID_t>& activeStates,
                            const hsmcpp::EventID_t event,
                            const VariantVector_t& args);

protected:
    void SetUp() override;
    void TearDown() override;

public:
    int mFailedTransitionCounter = 0;
    hsmcpp::EventID_t mLastFailedTransition;
    VariantVector_t mLastFailedTransitionArgs;
};

#endif  // HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP