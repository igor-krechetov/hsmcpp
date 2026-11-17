#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestsCommon.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    configureGTest("singlethread");

    int rc = RUN_ALL_TESTS();

    cleanupGTest();

    return rc;
}
