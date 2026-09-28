#include <cstddef>
#include <liblltools/allocator.hpp>
#include <unityboot/system.hpp>
#include <unityboot/memory.hpp>

namespace UnityBoot::Memory {
    extern "C" {
        extern uint8_t __heap_start[];
        extern uint8_t __heap_end[];
    }

    static LLTools::FirstFitAllocator heapAlloc{};
    static bool freezeFlag = false;

    void initialize() {
        uintptr_t heapStartAddr = reinterpret_cast<uintptr_t>(__heap_start);
        uintptr_t heapEndAddr = reinterpret_cast<uintptr_t>(__heap_end);

        heapAlloc.initialize(heapStartAddr, heapEndAddr, minBlockSize, heapAlignment);
    }

    void* allocate(size_t size) {
        if (freezeFlag) System::panic("UnityBoot::Memory::allocate()", "Memory is frozen");

        return heapAlloc.allocate(size);
    }

    void deallocate(void* ptr) {
        if (freezeFlag) System::panic("UnityBoot::Memory::deallocate()", "Memory is frozen");

        heapAlloc.deallocate(static_cast<uint8_t*>(ptr));
    }

    void freeze() {
        freezeFlag = true;
    }

    MemoryRegion getFreeMemoryRegion() {
        LLTools::MemoryBlockHeader* mbh = heapAlloc.getLastBlock();

        return MemoryRegion{reinterpret_cast<uintptr_t>(mbh), reinterpret_cast<uintptr_t>(mbh) + mbh->size + sizeof(LLTools::MemoryBlockHeader)};
    }
}