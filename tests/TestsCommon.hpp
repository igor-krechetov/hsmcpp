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

#endif // __HSMCPP_TESTS_TESTSCOMMON_HPP__