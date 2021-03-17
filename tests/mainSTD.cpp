#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "logging.hpp"
#include "TestsCommon.hpp"

__TRACE_PREINIT__();

int main(int argc, char** argv)
{
    __TRACE_INIT__();

    ::testing::InitGoogleMock(&argc, argv);
    configureGTest();

    return RUN_ALL_TESTS();
}