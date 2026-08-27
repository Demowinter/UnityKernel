#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
    void* memset(void* dest, uint8_t byte, size_t size);
    void* memcpy(void* dest, const void* src, size_t size);
    void* memmove(void* dest, const void* src, size_t size);
    void* malloc(size_t size);
    void free(void* ptr);
    int memcmp(const void* ptr1, const void* ptr2, size_t size);
    void* memchr(const void* ptr, int value, size_t size);
    void* calloc(size_t num, size_t size);
}