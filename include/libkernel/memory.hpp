#pragma once
#include <cstddef>
#include <cstdint>

namespace Kernel::Memory {
    constexpr size_t heapAlignment = 16;
    constexpr size_t minBlockSize = 32;

    void initialize();

    void* allocate(size_t size);
    void deallocate(void* ptr);
}