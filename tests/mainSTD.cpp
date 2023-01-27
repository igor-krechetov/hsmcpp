#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "TestsCommon.hpp"

int main(int argc, char** argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    configureGTest("std");

    RUN_ALL_TESTS();

    return 0;
}