#pragma once
#include <cstddef>
#include <cstdint>

namespace Kernel::Heap {
    constexpr size_t heapAlignment = 16;
    constexpr size_t minBlockSize = 32;

    void initialize(uintptr_t heapStartAddr, uintptr_t heapEndAddr);
    bool initialized();

    void* allocate(size_t size);
    void deallocate(void* ptr);

    void freeze();
    void defrost();
    bool frozen();
}