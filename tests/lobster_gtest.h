// LOBSTER GTest tracing support header.
//
// Originally authored by Florian Schanda (2023) and released to the PUBLIC
// DOMAIN. No other parts of LOBSTER are public domain — only this file.
//
// Usage inside a GTest body links the test to one or more requirement UIDs:
//
//   #include "lobster_gtest.h"
//
//   TEST_F(TrafficLightHsm, simple_transition) {
//       LOBSTER_TRACE("HSMCPP.SWR_HSM_020");   // requirement this test verifies
//       // ... test body unchanged ...
//   }
//
// The macros expand to GTest RecordProperty() calls that embed tracing
// metadata into the JUnit/GTest XML output — zero runtime cost, no change to
// test logic. lobster-gtest then reads that XML to build the trace evidence.

#ifndef HSMCPP_LOBSTER_GTEST_SUPPORT_H
#define HSMCPP_LOBSTER_GTEST_SUPPORT_H

#define LOBSTER_TRACE(prop)                                          \
    ::testing::Test::RecordProperty("lobster-tracing", (prop)),      \
        ::testing::Test::RecordProperty("lobster-tracing-file", __FILE__), \
        ::testing::Test::RecordProperty("lobster-tracing-line", __LINE__)

#define LOBSTER_EXCLUDE(reason)                                      \
    ::testing::Test::RecordProperty("lobster-exclude", (reason)),    \
        ::testing::Test::RecordProperty("lobster-tracing-file", __FILE__), \
        ::testing::Test::RecordProperty("lobster-tracing-line", __LINE__)

#endif  // HSMCPP_LOBSTER_GTEST_SUPPORT_H
