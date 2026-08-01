#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
    void* memset(void* dest, uint8_t byte, size_t size);
    void* memcpy(void* dest, const void* src, size_t size);
    void* memmove(void* dest, const void* src, size_t size);

    uintptr_t align(uintptr_t value, uint16_t alignment);
}