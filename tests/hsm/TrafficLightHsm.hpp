#ifndef __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__
#define __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__

#include "TestsCommon.hpp"
#include "hsm.hpp"

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
                     , public HierarchicalStateMachine<TrafficLightState, TrafficLightEvent, TrafficLightHsm>
{
public:
    TrafficLightHsm();
    virtual ~TrafficLightHsm();

    void setupDefault();

    void onNextStateTransition(const VariantList_t& args){}
    void onTurnOffTransition(const VariantList_t& args){ ++mStateCounterTurnOff; printf("----> onTurnOffTransition\n"); }

    void onOff(const VariantList_t& args){ ++mStateCounterOff; printf("----> OFF\n"); }
    void onStarting(const VariantList_t& args){ ++mStateCounterStarting; printf("----> onStarting\n"); }
    void onRed(const VariantList_t& args){ ++mStateCounterRed; printf("----> onRed\n"); }
    void onYellow(const VariantList_t& args){ ++mStateCounterYellow; printf("----> onYellow\n"); }
    void onGreen(const VariantList_t& args){ ++mStateCounterGreen; printf("----> onGreen\n"); }

    void onDummyAction(const VariantList_t& args){ ++mStateCounterDummy; printf("----> DUMMY\n"); }
    void onArgsTest(const VariantList_t& args){ ++mStateCounterArgsTest; printf("----> ArgsTest\n"); mArgsTest = args; }
    bool onExitCancel(){ ++mStateCounterOnExit; printf("----> onExitCancel\n"); return false; }
    bool onEnterCancel(const VariantList_t& args){ ++mStateCounterOnEnter; printf("----> onEnterCancel\n"); mArgsTest = args; return false; }
    bool onExit(){ ++mStateCounterOnExit; printf("----> onExit\n"); return true; }
    bool onEnter(const VariantList_t& args){ ++mStateCounterOnEnter; printf("----> onEnter\n"); mArgsTest = args; return true; }

protected:
    void SetUp() override;
    void TearDown() override;

public:
    int mStateCounterOff = 0;
    int mStateCounterStarting = 0;
    int mStateCounterRed = 0;
    int mStateCounterYellow = 0;
    int mStateCounterGreen = 0;
    int mStateCounterDummy = 0;
    int mStateCounterArgsTest = 0;
    int mStateCounterOnExit = 0;
    int mStateCounterOnEnter = 0;
    int mStateCounterTurnOff = 0;
    VariantList_t mArgsTest;
};

#endif // __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__