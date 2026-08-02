#pragma once
#include <cstdint>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment);
    uint64_t alignDown(uint64_t value, uint64_t alignment);
    bool isPowerOfTwo(uint64_t value);
    uint64_t alignUpPow2(uint64_t value, uint64_t alignment);
    uint64_t alignDownPow2(uint64_t value, uint64_t alignment);
    uint64_t roundUpDiv(uint64_t value, uint64_t divisor);
    uint64_t roundDownDiv(uint64_t value, uint64_t divisor);
    uint64_t minUint64(uint64_t a, uint64_t b);
    uint64_t maxUint64(uint64_t a, uint64_t b);
    uint64_t clampUint64(uint64_t value, uint64_t min, uint64_t max);
}