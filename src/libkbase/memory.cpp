#include <cstddef>
#include <cstdint>
#include <libkbase/memory.hpp>

extern "C" {
    void* memset(void* dest, uint8_t byte, size_t size) {
        for (size_t i = 0; i < size; i++) reinterpret_cast<uint8_t*>(dest)[i] = byte;

        return dest;
    }

    void* memcpy(void* dest, const void* src, size_t size) {
        for (size_t i = 0; i < size; i++) reinterpret_cast<uint8_t*>(dest)[i] = reinterpret_cast<const uint8_t*>(src)[i];

        return dest;
    }

    void* memmove(void* dest, const void* src, size_t size) {
        if (dest == src) return dest;
        if (dest < src) {
            for (size_t i = 0; i < size; i++) reinterpret_cast<uint8_t*>(dest)[i] = reinterpret_cast<const uint8_t*>(src)[i];
        } else {
            for (size_t i = size; i > 0; i--) reinterpret_cast<uint8_t*>(dest)[i - 1] = reinterpret_cast<const uint8_t*>(src)[i - 1];
        }
        return dest;
    }
}