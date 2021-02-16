#ifndef __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__
#define __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__

#include "TestsCommon.hpp"
#include "hsm.hpp"

enum class TrafficLightState
{
    OFF,
    STARTING,
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
                     , public HierarchicalStateMachine<TrafficLightState, TrafficLightEvent, TrafficLightHsm>
{
protected:
    void SetUp() override;
    void TearDown() override;

public:
    TrafficLightHsm() : HierarchicalStateMachine(TrafficLightState::OFF)
    {
    }

    virtual ~TrafficLightHsm(){}

    void setupDefault()
    {
        registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, nullptr, nullptr);
        registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, nullptr, nullptr);
        registerState(TrafficLightState::RED, this, &TrafficLightHsm::onRed, nullptr, nullptr);
        registerState(TrafficLightState::YELLOW, this, &TrafficLightHsm::onYellow, nullptr, nullptr);
        registerState(TrafficLightState::GREEN, this, &TrafficLightHsm::onGreen, nullptr, nullptr);

        registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);
        registerTransition(TrafficLightState::STARTING, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
        registerTransition(TrafficLightState::RED, TrafficLightState::YELLOW, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
        registerTransition(TrafficLightState::YELLOW, TrafficLightState::GREEN, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
        registerTransition(TrafficLightState::GREEN, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
    }

    void onNextStateTransition(const VariantList_t& args)
    {
    }

    void onOff(const VariantList_t& args){ ++mStateCounterOff; printf("----> OFF\n"); }
    void onStarting(const VariantList_t& args){ ++mStateCounterStarting; printf("----> onStarting\n"); }
    void onRed(const VariantList_t& args){ ++mStateCounterRed; printf("----> onRed\n"); }
    void onYellow(const VariantList_t& args){ ++mStateCounterYellow; printf("----> onYellow\n"); }
    void onGreen(const VariantList_t& args){ ++mStateCounterGreen; printf("----> onGreen\n"); }

    void onDummyAction(const VariantList_t& args){ ++mStateCounterDummy; printf("----> DUMMY\n"); }
    void onArgsTest(const VariantList_t& args){ ++mStateCounterArgsTest; printf("----> ArgsTest\n"); mArgsTest = args; }

public:
    int mStateCounterOff = 0;
    int mStateCounterStarting = 0;
    int mStateCounterRed = 0;
    int mStateCounterYellow = 0;
    int mStateCounterGreen = 0;
    int mStateCounterDummy = 0;
    int mStateCounterArgsTest = 0;
    VariantList_t mArgsTest;
};

#endif // __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__