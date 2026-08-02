#pragma once
#include <cstdint>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment);
    uint64_t alignDown(uint64_t value, uint64_t alignment);

    bool inRangeII(int64_t value, int64_t min, int64_t max);
    bool inRangeEI(int64_t value, int64_t min, int64_t max);
    bool inRangeIE(int64_t value, int64_t min, int64_t max);
    bool inRangeEE(int64_t value, int64_t min, int64_t max);
}