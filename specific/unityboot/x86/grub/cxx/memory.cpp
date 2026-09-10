#include <cstddef>
#include <liblltools/allocator.hpp>
#include <unityboot/memory.hpp>

namespace UnityBoot::Memory {
    extern "C" {
        extern uint8_t __heap_start[];
        extern uint8_t __heap_end[];
    }

    static LLTools::FirstFitAllocator heapAlloc{};

    void initialize() {
        uintptr_t heapStartAddr = reinterpret_cast<uintptr_t>(__heap_start);
        uintptr_t heapEndAddr = reinterpret_cast<uintptr_t>(__heap_end);

        heapAlloc.initialize(heapStartAddr, heapEndAddr, minBlockSize, heapAlignment);
    }

    void* allocate(size_t size) {
        return heapAlloc.allocate(size);
    }

    void deallocate(void* ptr) {
        heapAlloc.deallocate(static_cast<uint8_t*>(ptr));
    }
}