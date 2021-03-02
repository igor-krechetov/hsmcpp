#ifndef __HSMCPP_TESTS_TESTSCOMMON_HPP__
#define __HSMCPP_TESTS_TESTSCOMMON_HPP__

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// ======================================================
// GTest namespace
using ::testing::NiceMock;
using ::testing::NaggyMock;
using ::testing::Mock;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::InSequence;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::DoDefault;
using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::ReturnRef;
using ::testing::ReturnRefOfCopy;
using ::testing::SaveArg;
using ::testing::_;

// ======================================================
#define TEST_DESCRIPTION(desc)                  RecordProperty("description", desc)

#define EXPECT_EQ_IF(cond, val1, val2)          if (cond){ EXPECT_EQ((val1), (val2)); }

#define EXPECT_CALL_ACTION(_obj_, _call_, ...)    \
        EXPECT_CALL(_obj_, _call_).WillRepeatedly(DoAll(_obj_.getDefaultAsyncAction(), __VA_ARGS__))

#define EXPECT_CALL_ACTION_ONCE(_obj_, _call_, ...)    \
        EXPECT_CALL(_obj_, _call_).WillOnce(DoAll(_obj_.getDefaultAsyncAction(), __VA_ARGS__))

// ======================================================
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

#define DEF_STATE_ACTION_IMPL(_state)                                   \
    int mStateCounter##_state = 0;                                      \
    VariantList_t mArgs##_state;                                        \
    void on##_state(const VariantList_t& args)                          \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        printf("----> on" #_state "\n");                                \
        mArgs##_state = args;                                           \
    }                                                                   \
    DEF_EXIT_ACTION_IMPL(_state##Exit, true)                            \
    DEF_ENTER_ACTION_IMPL(_state##Enter, true)

#endif // __HSMCPP_TESTS_TESTSCOMMON_HPP__