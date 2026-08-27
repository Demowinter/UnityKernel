#pragma once
#include <cstdint>

extern "C" {
    bool isPowerOfTwo(uint64_t value);
    uint64_t roundUpDiv(uint64_t value, uint64_t divisor);
    uint64_t roundDownDiv(uint64_t value, uint64_t divisor);
    uint64_t minUint64(uint64_t a, uint64_t b);
    uint64_t maxUint64(uint64_t a, uint64_t b);
    uint64_t clampUint64(uint64_t value, uint64_t min, uint64_t max);
}