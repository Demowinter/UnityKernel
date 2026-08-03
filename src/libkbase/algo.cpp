#include <cstdint>
#include <libkbase/algo.hpp>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment) {
        return value + (alignment - (value % alignment)) % alignment;
    }

    uint64_t alignDown(uint64_t value, uint64_t alignment) {
        return value - (value % alignment);
    }
    
    uint64_t alignUpPow2(uint64_t value, uint64_t alignment) {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    uint64_t alignDownPow2(uint64_t value, uint64_t alignment) {
        return value & ~(alignment - 1);
    }

    bool inRangeII(int64_t value, int64_t min, int64_t max) {
        return value >= min && value <= max;
    }

    bool inRangeEI(int64_t value, int64_t min, int64_t max) {
        return value > min && value <= max;
    }

    bool inRangeIE(int64_t value, int64_t min, int64_t max) {
        return value >= min && value < max;
    }

    bool inRangeEE(int64_t value, int64_t min, int64_t max) {
        return value > min && value < max;
    }
}