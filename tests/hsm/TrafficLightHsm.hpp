#ifndef __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__
#define __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__

#include "TestsCommon.hpp"
#include "hsmcpp/hsm.hpp"

#undef __TRACE_CLASS__
#define __TRACE_CLASS__                         "TrafficLightHsm"

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

    bool checkConditionOff2Off(const VariantList_t& args);
    bool checkConditionOff2On(const VariantList_t& args);

protected:
    void SetUp() override;
    void TearDown() override;
};

#endif // __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__