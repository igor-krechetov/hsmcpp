// Copyright (C) 2021 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
//
// Tests for src/HsmImpl.cpp debug-logging paths and the src/hsm.cpp
// enable/disable debugging wrappers. All cases here are compile-gated by
// HSMBUILD_DEBUGGING (enableHsmDebugging / logHsmAction are #ifdef'd on that
// flag in production source); they compile out rather than fail when the flag
// is off. The flag defaults ON and the coverage build passes it explicitly.
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"
#include "utils/RandomGenerators.hpp"
#include "utils/ScopedEnv.hpp"
#include "utils/ScopedFileGuard.hpp"
#include "utils/TempHsmLog.hpp"

#ifdef HSMBUILD_DEBUGGING

// src/HsmImpl.cpp: enableHsmDebugging() default-path branch.
// With HSMCPP_DUMP_PATH unset, the no-arg enableHsmDebugging() must fall back to
// the compiled-in default path "./dump.hsmlog". Drive one transition so a record
// is written, then assert the default file exists and is non-empty.
TEST_F(ABCHsm, debug_enable_default_path) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_099");
    TEST_DESCRIPTION("enableHsmDebugging() with no path and no HSMCPP_DUMP_PATH writes to the default ./dump.hsmlog");

    //-------------------------------------------
    // PRECONDITIONS
    const std::string defaultPath = "./dump.hsmlog";
    // Ensure the env override is absent so the default-path branch is taken, and
    // guarantee the default file is removed on teardown even on assertion failure.
    ScopedEnv noDumpPath("HSMCPP_DUMP_PATH");
    ScopedFileGuard defaultLog(defaultPath);

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(enableHsmDebugging());
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    disableHsmDebugging();  // flush/close the log before reading its size

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(std::filesystem::exists(defaultPath));
    EXPECT_GT(std::filesystem::file_size(defaultPath), 0U);
}

// src/HsmImpl.cpp: enableHsmDebugging(path) explicit-path branch.
// When an explicit path is passed, the log must be written there rather than to
// the default or env-selected path. Drive one transition so a record is written,
// then assert the TempHsmLog file received non-empty output.
TEST_F(ABCHsm, debug_enable_explicit_path) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_099");
    TEST_DESCRIPTION("enableHsmDebugging(path) writes the execution log to the explicitly provided path");

    //-------------------------------------------
    // PRECONDITIONS
    // TempHsmLog picks a unique temp path and removes any pre-existing file in its
    // ctor (forcing the new-log branch) and deletes it on scope exit.
    TempHsmLog log;

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(enableHsmDebugging(log.path()));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    disableHsmDebugging();  // flush/close the log before reading its size

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(log.exists());
    EXPECT_GT(log.size(), 0U);
}

// src/HsmImpl.cpp: enableHsmDebugging() env-path branch.
// With HSMCPP_DUMP_PATH set, the no-arg enableHsmDebugging() must select the
// env-named file rather than the compiled-in default. Point the env var at a
// TempHsmLog path, call the no-arg overload, drive one transition so a record is
// written, then assert the env-named file received non-empty output.
TEST_F(ABCHsm, debug_enable_env_path) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_099");
    TEST_DESCRIPTION("enableHsmDebugging() with HSMCPP_DUMP_PATH set writes the log to the env-named path");

    //-------------------------------------------
    // PRECONDITIONS
    // TempHsmLog picks a unique temp path and removes any pre-existing file in its
    // ctor (forcing the new-log branch) and deletes it on scope exit. ScopedEnv points
    // HSMCPP_DUMP_PATH at that path for the life of the test and restores it afterwards.
    TempHsmLog log;
    ScopedEnv dump("HSMCPP_DUMP_PATH", log.path());

    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);
    initializeHsm();

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(enableHsmDebugging());
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    disableHsmDebugging();  // flush/close the log before reading its size

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(log.exists());
    EXPECT_GT(log.size(), 0U);
}

namespace {

// Split the structured log into individual records. Each record is emitted by
// logHsmAction() prefixed with "\n-\n" (see src/HsmImpl.cpp). Returns the raw
// text of every record (without the leading delimiter).
std::vector<std::string> splitLogRecords(const std::string& content) {
    std::vector<std::string> records;
    const std::string delimiter = "\n-\n";
    size_t pos = content.find(delimiter);

    while (std::string::npos != pos) {
        const size_t start = pos + delimiter.size();
        const size_t nextPos = content.find(delimiter, start);
        records.push_back(content.substr(start, (std::string::npos == nextPos) ? std::string::npos : (nextPos - start)));
        pos = nextPos;
    }

    return records;
}

}  // namespace

// src/HsmImpl.cpp: logHsmAction() structured-record body (PROPERTY — Property 1).
// For >=100 generated argument arrays, drive a transition carrying the args into a
// logged transition callback, read the log back, and assert the record round-trips
// its structured fields: a timestamp, the active state name, the event/target_state
// name, and every arg's toString() under "args:". An external self-transition on A
// keeps the HSM in a single, well-known state so each iteration produces one fresh
// TRANSITION record without needing to re-initialize the fixture.
TEST_F(ABCHsm, debug_record_preserves_args_and_metadata) {
    // SWR_HSM_096 confirmed present in requirements/software_requirements.trlc
    // (Structured Execution Log Format). The arg round-trip also exercises
    // SWR_HSM_050 (Transition Data Passing); 096 is linked primarily.
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_096");
    TEST_DESCRIPTION("logHsmAction records round-trip timestamp, active state, event/target, and each arg's toString()");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    // External self-transition on A: every E1 logs one TRANSITION record carrying
    // the supplied args while the HSM stays in A, so the loop needs no re-init.
    registerSelfTransition<ABCHsm>(AbcState::A,
                                   AbcEvent::E1,
                                   TransitionType::EXTERNAL_TRANSITION,
                                   this,
                                   &ABCHsm::onE1Transition);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    const std::string stateName = getStateName(AbcState::A);   // "A"
    const std::string eventName = getEventName(AbcEvent::E1);  // "E1"

    //-------------------------------------------
    // ACTIONS + VALIDATION (>=100 generated iterations)
    constexpr unsigned int kIterations = 100U;

    for (unsigned int seed = 0U; seed < kIterations; ++seed) {
        VariantVector_t args = makeRandomArgs(seed);

        // Fresh TempHsmLog per iteration: its ctor removes any prior file (forcing
        // the new-log branch) and its dtor deletes it, so each round inspects a log
        // containing only this iteration's records.
        TempHsmLog log;
        ASSERT_TRUE(enableHsmDebugging(log.path())) << "seed=" << seed;

        mTransitionCounterE1 = 0;
        // Synchronous, array-based transition: carries the generated args verbatim
        // and blocks until the transition (and its log record) are complete.
        ASSERT_TRUE(transitionExWithArgsArray(AbcEvent::E1,
                                              false /*clearQueue*/,
                                              true /*sync*/,
                                              TIMEOUT_SYNC_TRANSITION,
                                              VariantVector_t(args)))
            << "seed=" << seed;
        ASSERT_EQ(mTransitionCounterE1, 1) << "seed=" << seed;

        disableHsmDebugging();  // flush/close the log before reading it back

        //--- round-trip assertions ---
        const std::string content = log.read();
        ASSERT_FALSE(content.empty()) << "seed=" << seed;

        const std::vector<std::string> records = splitLogRecords(content);
        ASSERT_FALSE(records.empty()) << "seed=" << seed;

        // Locate the TRANSITION record for this self-transition. The human-readable
        // state/event NAME fields (from_state/target_state/event/active_states) are
        // resolved via the Impl's getStateName()/getEventName(), which only consult
        // the registered names when debug traces are enabled; under
        // HSM_LOGGING_MODE_OFF (HSM_DISABLE_DEBUG_TRACES) they return "" by design.
        // So the record is identified by its action and the args it carries (the
        // args are serialized directly via Variant::toString(), independent of the
        // name-resolution path), not by a name field that may legitimately be empty.
        const std::string* transitionRecord = nullptr;
        for (auto it = records.rbegin(); it != records.rend(); ++it) {
            if (std::string::npos != it->find("action: transition")) {
                transitionRecord = &(*it);
                break;
            }
        }
        ASSERT_NE(transitionRecord, nullptr) << "seed=" << seed << " content=\n" << content;

        // timestamp field present (always populated)
        EXPECT_NE(transitionRecord->find("timestamp: \""), std::string::npos) << "seed=" << seed;

  #ifndef HSM_DISABLE_DEBUG_TRACES
        // The structured NAME fields are only resolved when debug traces are
        // enabled. Assert them only in that configuration; the arg round-trip below
        // is checked unconditionally.
        const std::string targetField = "target_state: \"" + stateName + "\"";
        // active_states entry naming the current state ("A")
        EXPECT_NE(transitionRecord->find("- \"" + stateName + "\""), std::string::npos) << "seed=" << seed;
        // event name present
        EXPECT_NE(transitionRecord->find("event: \"" + eventName + "\""), std::string::npos) << "seed=" << seed;
        // target_state name present
        EXPECT_NE(transitionRecord->find(targetField), std::string::npos) << "seed=" << seed;
  #else
        (void)stateName;
        (void)eventName;
  #endif  // HSM_DISABLE_DEBUG_TRACES

        // Every arg's toString() appears under "args:" as a "- <value>" entry.
        const size_t argsPos = transitionRecord->find("args:");
        ASSERT_NE(argsPos, std::string::npos) << "seed=" << seed;
        const std::string argsSection = transitionRecord->substr(argsPos);
        for (size_t i = 0U; i < args.size(); ++i) {
            const std::string expectedEntry = "- " + args[i].toString();
            EXPECT_NE(argsSection.find(expectedEntry), std::string::npos)
                << "seed=" << seed << " argIndex=" << i << " expected='" << expectedEntry << "'\nargs section:\n"
                << argsSection;
        }
    }
}

// src/hsm.cpp: enableHsmDebugging(path) / disableHsmDebugging() public wrappers.
// The enable/disable toggle must have an observable effect on the execution log:
// while enabled, a transition grows the log file; after disableHsmDebugging(), a
// further transition must NOT grow it (recording has stopped and the log is
// closed). An external self-transition on A keeps the HSM in a single, well-known
// state so each E1 drives one TRANSITION record without re-initializing.
TEST_F(ABCHsm, public_enable_disable_toggle) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_099");
    TEST_DESCRIPTION("enableHsmDebugging grows the log on a transition; disableHsmDebugging stops further growth");

    //-------------------------------------------
    // PRECONDITIONS
    // TempHsmLog picks a unique temp path and removes any pre-existing file in its
    // ctor (forcing the new-log branch) and deletes it on scope exit.
    TempHsmLog log;

    registerState<ABCHsm>(AbcState::A);
    // External self-transition on A: every E1 logs one TRANSITION record while the
    // HSM stays in A, so the toggle can be exercised with repeated events.
    registerSelfTransition(AbcState::A, AbcEvent::E1, TransitionType::EXTERNAL_TRANSITION);
    initializeHsm();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS + VALIDATION
    // Enable debugging, drive a transition; the log must grow beyond empty.
    ASSERT_TRUE(enableHsmDebugging(log.path()));
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    // disableHsmDebugging() flushes/closes the log; capture the grown size.
    disableHsmDebugging();
    ASSERT_TRUE(log.exists());
    const std::uintmax_t sizeAfterEnable = log.size();
    EXPECT_GT(sizeAfterEnable, 0U);

    // With debugging disabled, a further transition must NOT grow the log file.
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    const std::uintmax_t sizeAfterDisable = log.size();
    EXPECT_EQ(sizeAfterDisable, sizeAfterEnable);
}

#endif  // HSMBUILD_DEBUGGING
