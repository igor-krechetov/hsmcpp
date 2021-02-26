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

#define DEF_STATE_ACTION_IMPL(_state)                                   \
    int mStateCounter##_state = 0;                                      \
    VariantList_t mArgs##_state;                                        \
    void on##_state(const VariantList_t& args)                          \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        printf("----> on" #_state "\n");                                \
        mArgs##_state = args;                                           \
    }

#define DEF_EXIT_ACTION_IMPL(_state, _ret)                              \
    int mStateCounter##_state = 0;                                      \
    bool on##_state()                                                   \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        printf("----> on" #_state "\n");                                \
        return (_ret);                                                  \
    }

#define DEF_ENTER_ACTION_IMPL(_state, _ret)                             \
    int mStateCounter##_state = 0;                                      \
    VariantList_t mArgs##_state;                                        \
    bool on##_state(const VariantList_t& args)                          \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        printf("----> on" #_state "\n");                                \
        mArgs##_state = args;                                           \
        return (_ret);                                                  \
    }

#define DEF_TRANSITION_IMPL(_name)                                     \
    int mTransitionCounter##_name = 0;                                 \
    VariantList_t mTransitionArgs##_name;                              \
    void on##_name##Transition(const VariantList_t& args)              \
    {                                                                  \
        ++mTransitionCounter##_name;                                   \
        printf("----> on" #_name "Transition\n");                      \
        mTransitionArgs##_name = args;                                 \
    }

class TrafficLightHsm: public testing::Test
                     , public HierarchicalStateMachine<TrafficLightState, TrafficLightEvent, TrafficLightHsm>
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

protected:
    void SetUp() override;
    void TearDown() override;
};

#endif // __HSMCPP_TESTS_HSM_TRAFFICLIGHTHSM_HPP__