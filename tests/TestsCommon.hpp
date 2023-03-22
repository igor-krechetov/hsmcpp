#ifndef HSMCPP_TESTS_TESTSCOMMON_HPP
#define HSMCPP_TESTS_TESTSCOMMON_HPP

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <list>
#include <string>
#include <memory>

#include "hsmcpp/IHsmEventDispatcher.hpp"
#include "hsmcpp/logging.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "TestsCommon"

// ======================================================
// GTest namespace
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::DoDefault;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::Mock;
using ::testing::NaggyMock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::ReturnRef;
using ::testing::ReturnRefOfCopy;
using ::testing::SaveArg;

using namespace hsmcpp;

// ======================================================
// Utility functions
void configureGTest(const std::string& name);

template <typename HsmStateEnum>
bool compareStateLists(const std::list<HsmStateEnum> l1, const std::list<HsmStateEnum> l2) {
    bool equalLists = false;

    if (l1.size() == l2.size()) {
        equalLists = true;

        for (auto it1 = l1.begin(); it1 != l1.end(); ++it1) {
            if (std::find(l2.begin(), l2.end(), *it1) == l2.end()) {
                equalLists = false;
                break;
            }
        }
    }

    if (false == equalLists) {
        HSM_TRACE_CALL_DEBUG_ARGS("states are different");

        HSM_TRACE_DEBUG("Expected:");
        for (auto it = l2.begin(); it != l2.end(); ++it) {
            HSM_TRACE_DEBUG("%d", SC2INT(*it));
        }

        HSM_TRACE_DEBUG("Was:");
        for (auto it = l1.begin(); it != l1.end(); ++it) {
            HSM_TRACE_DEBUG("%d", SC2INT(*it));
        }
    }

    return equalLists;
}

// ======================================================
#define TEST_DESCRIPTION(desc) ::testing::Test::RecordProperty("description", desc)

#define EXPECT_EQ_IF(cond, val1, val2) \
  if (cond) {                          \
    EXPECT_EQ((val1), (val2));         \
  }

#define EXPECT_CALL_ACTION(_obj_, _call_, ...) \
  EXPECT_CALL(_obj_, _call_).WillRepeatedly(DoAll(_obj_.getDefaultAsyncAction(), __VA_ARGS__))

#define EXPECT_CALL_ACTION_ONCE(_obj_, _call_, ...) \
  EXPECT_CALL(_obj_, _call_).WillOnce(DoAll(_obj_.getDefaultAsyncAction(), __VA_ARGS__))

// ======================================================
// HSM initialization

extern std::shared_ptr<hsmcpp::IHsmEventDispatcher> gDispatcher;

// since unit tests are executed on a separate thread we need a way to create/destroy dispatchers on main thread (in a sync way)
// this function blocks thread execution until func() is finished running on main thread.
// implementation depends on used dispatcher
bool executeOnMainThread(std::function<bool()> func);

#if defined(TEST_HSM_GLIB)
  #include "hsmcpp/HsmEventDispatcherGLib.hpp"

  #define CREATE_DISPATCHER() HsmEventDispatcherGLib::create()
#elif defined(TEST_HSM_GLIBMM)
  #include "hsmcpp/HsmEventDispatcherGLibmm.hpp"

  #define CREATE_DISPATCHER() HsmEventDispatcherGLibmm::create()
#elif defined(TEST_HSM_STD)
  #include "hsmcpp/HsmEventDispatcherSTD.hpp"

  #define CREATE_DISPATCHER() HsmEventDispatcherSTD::create()
#elif defined(TEST_HSM_QT)
  #include "hsmcpp/HsmEventDispatcherQt.hpp"

  #define CREATE_DISPATCHER() HsmEventDispatcherQt::create()
#elif defined(TEST_HSM_FREERTOS)
  #include "hsmcpp/HsmEventDispatcherFreeRTOS.hpp"

    // NOTE: priority should be higher than priority of the task where gTest is running
  #define CREATE_DISPATCHER() HsmEventDispatcherFreeRTOS::create(configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 1)
#else
  #error HSM Dispatcher not specified
#endif

#define INITIALIZE_HSM()                                                                        \
  ASSERT_TRUE(executeOnMainThread([this]() {                                                    \
    if (!gDispatcher){                                                                          \
      gDispatcher = std::static_pointer_cast<hsmcpp::IHsmEventDispatcher>(CREATE_DISPATCHER()); \
    }                                                                                           \
    return initialize(gDispatcher);                                                             \
  }))

#define RELEASE_HSM()                     \
  ASSERT_TRUE(executeOnMainThread([this]() { \
return true;                              \
  }))

// ======================================================
#define DEF_EXIT_ACTION_IMPL(_state, _ret)        \
  int mStateCounter##_state = 0;                  \
  bool on##_state() {                             \
    ++mStateCounter##_state;                      \
    HSM_TRACE_CALL_ARGS("----> on" #_state "\n"); \
    return (_ret);                                \
  }

#define DEF_ENTER_ACTION_IMPL(_state, _ret)       \
  int mStateCounter##_state = 0;                  \
  VariantVector_t mArgs##_state;                  \
  bool on##_state(const VariantVector_t& args) {  \
    ++mStateCounter##_state;                      \
    HSM_TRACE_CALL_ARGS("----> on" #_state "\n"); \
    mArgs##_state = args;                         \
    return (_ret);                                \
  }

#define DEF_TRANSITION_IMPL(_name)                          \
  int mTransitionCounter##_name = 0;                        \
  VariantVector_t mTransitionArgs##_name;                   \
  void on##_name##Transition(const VariantVector_t& args) { \
    ++mTransitionCounter##_name;                            \
    HSM_TRACE_CALL_ARGS("----> on" #_name "Transition\n");  \
    mTransitionArgs##_name = args;                          \
  }

#define DEF_STATE_ACTION_IMPL(_state)             \
  int mStateCounter##_state = 0;                  \
  VariantVector_t mArgs##_state;                  \
  void on##_state(const VariantVector_t& args) {  \
    ++mStateCounter##_state;                      \
    HSM_TRACE_CALL_ARGS("----> on" #_state "\n"); \
    mArgs##_state = args;                         \
  }                                               \
  DEF_EXIT_ACTION_IMPL(_state##Exit, true)        \
  DEF_EXIT_ACTION_IMPL(_state##ExitCancel, false) \
  DEF_ENTER_ACTION_IMPL(_state##Enter, true)      \
  DEF_ENTER_ACTION_IMPL(_state##EnterCancel, false)

#endif  // HSMCPP_TESTS_TESTSCOMMON_HPP