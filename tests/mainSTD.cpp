#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestsCommon.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    configureGTest("std");

    int rc = RUN_ALL_TESTS();

    // NOTE: return 0 to avoid stoppic CI action
    return 0;
}