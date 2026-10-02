#include <cstddef>
#include <cstdint>
#include <liblltools/allocator.hpp>
#include <kernel/system.hpp>
#include <kernel/heap.hpp>

namespace Kernel::Heap {
    static LLTools::FirstFitAllocator heapAlloc{};

    static bool initFlag = false;
    static bool freezeFlag = false;

    void initialize(uintptr_t heapStartAddr, uintptr_t heapEndAddr) {
        if (!initFlag) initFlag = heapAlloc.initialize(heapStartAddr, heapEndAddr, minBlockSize, heapAlignment);
    }

    bool initialized() {
        return initFlag;
    }

    void* allocate(size_t size) {
        if (!initFlag) System::panic("Kernel::Heap::allocate()", "Heap is not initialized");
        if (freezeFlag) System::panic("Kernel::Heap::allocate()", "Heap is frozen");

        return heapAlloc.allocate(size);
    }

    void deallocate(void* ptr) {
        if (!initFlag) System::panic("Kernel::Heap::allocate()", "Heap is not initialized");
        if (freezeFlag) System::panic("Kernel::Heap::deallocate()", "Heap is frozen");

        heapAlloc.deallocate(static_cast<uint8_t*>(ptr));
    }

    void freeze() {
        freezeFlag = true;
    }

    void defrost() {
        freezeFlag = false;
    }

    bool frozen() {
        return freezeFlag;
    }
}