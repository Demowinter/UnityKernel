#include <cstdint>
#include <libkbase/math.hpp>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment) {
        return value + (alignment - (value % alignment)) % alignment;
    }

    uint64_t alignDown(uint64_t value, uint64_t alignment) {
        return value - (value % alignment);
    }
}