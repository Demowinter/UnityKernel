#include <cstddef>
#include <cstdint>
#include <libkernel/memory.hpp>
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

    void* malloc(size_t size) {
        return Kernel::Memory::allocate(size);
    }

    void free(void* ptr) {
        Kernel::Memory::deallocate(ptr);
    }

    int memcmp(const void* ptr1, const void* ptr2, size_t size) {
        for (size_t i = 0; i < size; i++) {
            uint8_t byte1 = reinterpret_cast<const uint8_t*>(ptr1)[i];
            uint8_t byte2 = reinterpret_cast<const uint8_t*>(ptr2)[i];

            if (byte1 != byte2) return static_cast<int>(byte1) - static_cast<int>(byte2);
        }

        return 0;
    }

    void* memchr(const void* ptr, int value, size_t size) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < size; i++) {
            if (bytes[i] == static_cast<uint8_t>(value)) {
                return const_cast<void*>(static_cast<const void*>(bytes + i));
            }
        }

        return nullptr;
    }

    void* calloc(size_t num, size_t size) {
        size_t total_size = num * size;
        void* ptr = malloc(total_size);
        if (ptr) {
            memset(ptr, 0, total_size);
        }
        return ptr;
    }
}