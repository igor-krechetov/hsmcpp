#ifndef __HSMCPP_TESTS_TESTSCOMMON_HPP__
#define __HSMCPP_TESTS_TESTSCOMMON_HPP__

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "hsmcpp/IHsmEventDispatcher.hpp"

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
// Utility functions
void configureGTest();

// ======================================================
#define TEST_DESCRIPTION(desc)                  RecordProperty("description", desc)

#define EXPECT_EQ_IF(cond, val1, val2)          if (cond){ EXPECT_EQ((val1), (val2)); }

#define EXPECT_CALL_ACTION(_obj_, _call_, ...)    \
        EXPECT_CALL(_obj_, _call_).WillRepeatedly(DoAll(_obj_.getDefaultAsyncAction(), __VA_ARGS__))

#define EXPECT_CALL_ACTION_ONCE(_obj_, _call_, ...)    \
        EXPECT_CALL(_obj_, _call_).WillOnce(DoAll(_obj_.getDefaultAsyncAction(), __VA_ARGS__))

// ======================================================
// HSM initialization

// since unit tests are executed on a separate thread we need a way to create/destroy dispatchers on main thread (in a sync way)
// this function blocks thread execution until func() is finished running on main thread.
// implementation depends on used dispatcher
bool executeOnMainThread(std::function<bool()> func);

#if defined(TEST_HSM_GLIB)
    #include "hsmcpp/HsmEventDispatcherGLib.hpp"

    #define CREATE_DISPATCHER()           std::make_shared<HsmEventDispatcherGLib>()
#elif defined(TEST_HSM_GLIBMM)
    #include "hsmcpp/HsmEventDispatcherGLibmm.hpp"

    #define CREATE_DISPATCHER()           std::make_shared<HsmEventDispatcherGLibmm>()
#elif defined(TEST_HSM_STD)
    #include "hsmcpp/HsmEventDispatcherSTD.hpp"

    #define CREATE_DISPATCHER()           std::make_shared<HsmEventDispatcherSTD>()
#else
    #error HSM Dispatcher not specified
#endif

#define INITIALIZE_HSM()                  ASSERT_TRUE(executeOnMainThread([&](){ return initialize(CREATE_DISPATCHER()); }))
#define RELEASE_HSM()                     ASSERT_TRUE(executeOnMainThread([&](){ release(); return true; }))

// ======================================================
#define DEF_EXIT_ACTION_IMPL(_state, _ret)                              \
    int mStateCounter##_state = 0;                                      \
    bool on##_state()                                                   \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        __TRACE_CALL_ARGS__("----> on" #_state "\n");                   \
        return (_ret);                                                  \
    }

#define DEF_ENTER_ACTION_IMPL(_state, _ret)                             \
    int mStateCounter##_state = 0;                                      \
    VariantList_t mArgs##_state;                                        \
    bool on##_state(const VariantList_t& args)                          \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        __TRACE_CALL_ARGS__("----> on" #_state "\n");                   \
        mArgs##_state = args;                                           \
        return (_ret);                                                  \
    }

#define DEF_TRANSITION_IMPL(_name)                                     \
    int mTransitionCounter##_name = 0;                                 \
    VariantList_t mTransitionArgs##_name;                              \
    void on##_name##Transition(const VariantList_t& args)              \
    {                                                                  \
        ++mTransitionCounter##_name;                                   \
        __TRACE_CALL_ARGS__("----> on" #_name "Transition\n");         \
        mTransitionArgs##_name = args;                                 \
    }

#define DEF_STATE_ACTION_IMPL(_state)                                   \
    int mStateCounter##_state = 0;                                      \
    VariantList_t mArgs##_state;                                        \
    void on##_state(const VariantList_t& args)                          \
    {                                                                   \
        ++mStateCounter##_state;                                        \
        __TRACE_CALL_ARGS__("----> on" #_state "\n");                   \
        mArgs##_state = args;                                           \
    }                                                                   \
    DEF_EXIT_ACTION_IMPL(_state##Exit, true)                            \
    DEF_ENTER_ACTION_IMPL(_state##Enter, true)

#endif // __HSMCPP_TESTS_TESTSCOMMON_HPP__