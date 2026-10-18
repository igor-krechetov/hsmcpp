#include "utils/RandomGenerators.hpp"

#include <limits>
#include <random>
#include <string>

using hsmcpp::Variant;
using hsmcpp::VariantVector_t;

VariantVector_t makeRandomArgs(const unsigned int seed) {
    // Minimal LCG (glibc constants) — reproducible without <random> global state.
    unsigned int state = (seed * 1103515245U) + 12345U;
    auto next = [&state]() -> unsigned int {
        state = (state * 1103515245U) + 12345U;
        return (state >> 16U) & 0x7FFFU;
    };

    const size_t count = 1U + (next() % 4U);  // 1..4 args, length varies by seed
    VariantVector_t args;
    args.reserve(count);

    for (size_t i = 0U; i < count; ++i) {
        switch (next() % 3U) {
            case 0U: {  // signed integer value
                args.push_back(Variant(static_cast<int64_t>(next()) - 16384));
                break;
            }
            case 1U: {  // string value
                args.push_back(Variant(std::string("s") + std::to_string(next())));
                break;
            }
            default: {  // nested vector value (exercises composite toString())
                VariantVector_t nested;
                nested.push_back(Variant(static_cast<int64_t>(next())));
                nested.push_back(Variant(std::string("n") + std::to_string(next())));
                args.push_back(Variant::make(nested));
                break;
            }
        }
    }

    return args;
}

std::vector<int32_t> makeRandomIds(const unsigned int seed, const int count) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int32_t> dist(std::numeric_limits<int32_t>::min(),
                                                std::numeric_limits<int32_t>::max());
    std::vector<int32_t> ids;
    ids.reserve(static_cast<size_t>(count));

    while (static_cast<int>(ids.size()) < count) {
        const int32_t id = dist(rng);
        // Exclude the invalid sentinels so every generated id is a "valid id".
        if ((id != hsmcpp::INVALID_HSM_STATE_ID) && (id != hsmcpp::INVALID_HSM_EVENT_ID)) {
            ids.push_back(id);
        }
    }

    return ids;
}
