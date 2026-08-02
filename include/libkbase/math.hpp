#pragma once
#include <cstdint>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment);
    uint64_t alignDown(uint64_t value, uint64_t alignment);
}