#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "logging.hpp"

__TRACE_PREINIT__();

int main(int argc, char** argv)
{
    __TRACE_INIT__();

    ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}