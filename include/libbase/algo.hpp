#pragma once
#include <cstdint>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment);
    uint64_t alignDown(uint64_t value, uint64_t alignment);
    uint64_t alignUpPow2(uint64_t value, uint64_t alignment);
    uint64_t alignDownPow2(uint64_t value, uint64_t alignment);

    bool inRangeII(int64_t value, int64_t min, int64_t max);
    bool inRangeEI(int64_t value, int64_t min, int64_t max);
    bool inRangeIE(int64_t value, int64_t min, int64_t max);
    bool inRangeEE(int64_t value, int64_t min, int64_t max);

    char* itoa(int64_t value, char* buffer, int base);
    char* utoa(uint64_t value, char* buffer, int base);
    char* ftoa(float value, char* buffer, int precision);
    char* ltoa(int64_t value, char* buffer, int base);
    char* ultoa(uint64_t value, char* buffer, int base);
}