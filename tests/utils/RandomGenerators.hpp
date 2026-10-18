// Deterministic, seed-reproducible generators for property-style tests.
// Shared by the manual-loop PROPERTY cases (no external PBT framework): a
// failing iteration can always be reproduced from its seed.
#ifndef HSMCPP_TESTS_UTILS_RANDOMGENERATORS_HPP
#define HSMCPP_TESTS_UTILS_RANDOMGENERATORS_HPP

#include <cstdint>
#include <vector>

#include "hsmcpp/HsmTypes.hpp"  // INVALID_HSM_STATE_ID / INVALID_HSM_EVENT_ID
#include "hsmcpp/variant.hpp"   // Variant / VariantVector_t

// Produces a VariantVector_t whose length (1..4) and element types/values are
// derived deterministically from the seed, mixing signed-int, string and nested
// vector Variants so the round-trip exercises heterogeneous and composite
// payloads (the nested vector exercises Variant's composite toString()).
hsmcpp::VariantVector_t makeRandomArgs(const unsigned int seed);

// Produces `count` valid HSM ids derived deterministically from the seed,
// spanning the full int32 range (negatives, zero, large positives) while
// excluding the INVALID_HSM_STATE_ID / INVALID_HSM_EVENT_ID sentinels.
std::vector<int32_t> makeRandomIds(const unsigned int seed, const int count);

#endif  // HSMCPP_TESTS_UTILS_RANDOMGENERATORS_HPP
