#pragma once
#include <cstddef>
#include <cstdint>

namespace Kernel::MemorySubsystem {
    // constexpr size_t heapAlignment = 16;
    // constexpr size_t minBlockSize = 32;

    void initialize(uintptr_t heapStartAddr, uintptr_t heapEndAddr);

    // void* allocateBasic(size_t size);
    // void deallocateBasic(void* ptr);
}