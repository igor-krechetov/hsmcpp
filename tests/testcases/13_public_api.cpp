// Copyright (C) 2021 Ihor Krechetov
// Distributed under MIT license. See file LICENSE for details
//
// Tests for src/hsm.cpp public API wrappers (dispatcher(), isInitialized(),
// isStateActive(), transitionWithArgsArray(), and the base
// getStateName()/getEventName() id->string mappings).
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"
#include "utils/RandomGenerators.hpp"


// on a plain HierarchicalStateMachine the base
// getStateName()/getEventName() implement the identity mapping
// id -> std::to_string(id) for any valid id, and yield "" for the
// INVALID_HSM_STATE_ID / INVALID_HSM_EVENT_ID sentinels.
//
// CRITICAL: this must use the BASE HierarchicalStateMachine class directly --
// ABCHsm (and other handler subclasses) override getStateName()/getEventName()
// to return registered human-readable names, which would defeat the purpose of
// this test. Hence a plain TEST(...) constructing a HierarchicalStateMachine
// instance rather than TEST_F(ABCHsm, ...).
//
// The valid ids are produced by the shared deterministic generator
// makeRandomIds(seed, count) (tests/utils/RandomGenerators.hpp). The property is
// checked over >=100 iterations with a manual loop (no rapidcheck).
namespace {

// getStateName()/getEventName() are protected on the base
// HierarchicalStateMachine, so they cannot be called through a base-class
// instance directly. This thin test-only subclass re-exposes them via `using`
// WITHOUT overriding them, so the base id->string identity mapping (the thing
// under test) is still what runs. Subclasses like ABCHsm instead override these
// to return registered names, which would defeat the test -- hence this minimal
// exposer rather than ABCHsm.
class BaseNameExposer : public hsmcpp::HierarchicalStateMachine {
public:
    explicit BaseNameExposer(const hsmcpp::StateID_t initialState)
        : hsmcpp::HierarchicalStateMachine(initialState) {}

    using hsmcpp::HierarchicalStateMachine::getEventName;
    using hsmcpp::HierarchicalStateMachine::getStateName;
};

}  // namespace

TEST(HsmPublicApi, public_base_name_mapping) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_100");
    TEST_DESCRIPTION(
        "the base HierarchicalStateMachine::getStateName()/getEventName() map any valid id to "
        "std::to_string(id) and yield \"\" for the INVALID_HSM_STATE_ID/INVALID_HSM_EVENT_ID sentinels");

    //-------------------------------------------
    // PRECONDITIONS
    // Use the BASE class mapping: ABCHsm and other handler subclasses override
    // these accessors with registered names, so only the plain
    // HierarchicalStateMachine implementation exercises the base id->string
    // mapping. BaseNameExposer re-exposes the protected base methods without
    // overriding them, so the base implementation is still what runs.
    BaseNameExposer hsm(0);

    //-------------------------------------------
    // ACTIONS / VALIDATION
    // The invalid sentinels must always map to the empty string.
    EXPECT_EQ(hsm.getStateName(hsmcpp::INVALID_HSM_STATE_ID), std::string());
    EXPECT_EQ(hsm.getEventName(hsmcpp::INVALID_HSM_EVENT_ID), std::string());

    // Also exercise a few fixed boundary ids alongside the generated ones.
    const int32_t fixedIds[] = {0, 1, -1, std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()};
    for (const int32_t id : fixedIds) {
        EXPECT_EQ(hsm.getStateName(id), std::to_string(id)) << "state name mismatch for fixed id " << id;
        EXPECT_EQ(hsm.getEventName(id), std::to_string(id)) << "event name mismatch for fixed id " << id;
    }

    // >=100 generated valid ids: each must map to std::to_string(id) for both
    // the state and event accessors.
    constexpr int kIterations = 128;
    const std::vector<int32_t> ids = makeRandomIds(1u, kIterations);
    ASSERT_GE(static_cast<int>(ids.size()), 100);

    for (size_t i = 0; i < ids.size(); ++i) {
        const int32_t id = ids[i];
        const std::string expected = std::to_string(id);

        // getStateName() treats its argument as a StateID_t, getEventName() as an
        // EventID_t; both are int32_t and share the identity mapping.
        EXPECT_EQ(hsm.getStateName(static_cast<hsmcpp::StateID_t>(id)), expected)
            << "state name mismatch at iteration " << i << " for id " << id;
        EXPECT_EQ(hsm.getEventName(static_cast<hsmcpp::EventID_t>(id)), expected)
            << "event name mismatch at iteration " << i << " for id " << id;
    }
}

TEST_F(ABCHsm, public_dispatcher_accessor) {
    TEST_DESCRIPTION("dispatcher() returns the dispatcher supplied at initialization");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);

    //-------------------------------------------
    // ACTIONS
    INITIALIZE_HSM();

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(dispatcher().lock(), gDispatcher);
}

TEST_F(ABCHsm, public_isinitialized_accessor) {
    TEST_DESCRIPTION("isInitialized() reports false before initialize() and true after");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    EXPECT_FALSE(isInitialized());

    //-------------------------------------------
    // ACTIONS
    INITIALIZE_HSM();

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(isInitialized());
}

TEST_F(ABCHsm, public_isstateactive_accessor) {
    TEST_DESCRIPTION("isStateActive() reports the active state true and inactive states false after a transition");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A);
    registerState<ABCHsm>(AbcState::B);
    registerTransition<ABCHsm>(AbcState::A, AbcState::B, AbcEvent::E1);

    INITIALIZE_HSM();
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::A}));

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(transitionSync(AbcEvent::E1, TIMEOUT_SYNC_TRANSITION));

    //-------------------------------------------
    // VALIDATION
    // oracle: getActiveStates() reports B active after A -> B
    ASSERT_TRUE(compareStateLists(getActiveStates(), {AbcState::B}));
    EXPECT_TRUE(isStateActive(AbcState::B));
    EXPECT_FALSE(isStateActive(AbcState::A));
}