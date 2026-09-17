#pragma once
#include <cstddef>

namespace UnityBoot::Memory {
    constexpr size_t heapAlignment = 16;
    constexpr size_t minBlockSize = 32;

    struct MemoryRegion {
        uintptr_t start;
        uintptr_t end;
    };

    void initialize();

    void* allocate(size_t size);
    void deallocate(void* ptr);

    void freeze();
    
    MemoryRegion getFreeMemoryRegion();
}