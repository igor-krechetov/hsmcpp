// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP
#define HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS                         "TrafficLightHsm"

enum class TrafficLightState
{
    OFF,
    STARTING,

    OPERABLE,

    RED,
    YELLOW,
    GREEN
};

enum class TrafficLightEvent
{
    TURN_ON,
    TURN_OFF,
    NEXT_STATE
};

class TrafficLightHsm: public testing::Test
                     , public HierarchicalStateMachine<TrafficLightState, TrafficLightEvent>
{
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

    void onTransitionFailed(const TrafficLightEvent event, const VariantVector_t& args);

protected:
    void SetUp() override;
    void TearDown() override;

public:
    int mFailedTransitionCounter = 0;
    TrafficLightEvent mLastFailedTransition;
    VariantVector_t mLastFailedTransitionArgs;
};

#endif // HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP