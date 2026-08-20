#include <cstdint>
#include <libkbase/math.hpp>

extern "C" {
    bool isPowerOfTwo(uint64_t value) {
        return value != 0 && (value & (value - 1)) == 0;
    }

    uint64_t roundUpDiv(uint64_t value, uint64_t divisor) {
        return (value + divisor - 1) / divisor;
    }

    uint64_t roundDownDiv(uint64_t value, uint64_t divisor) {
        return value / divisor;
    }

    uint64_t minUint64(uint64_t a, uint64_t b) {
        return a < b ? a : b;
    }

    uint64_t maxUint64(uint64_t a, uint64_t b) {
        return a > b ? a : b;
    }

    uint64_t clampUint64(uint64_t value, uint64_t min, uint64_t max) {
        return minUint64(maxUint64(value, min), max);
    }
}