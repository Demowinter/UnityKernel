#pragma once
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace LLTools {
    constexpr uint64_t memoryBlockMagic = 0xF5CD3EE914E9D29F;

    constexpr uint8_t used = 0x01;
    constexpr uint8_t reserved1 = 0x02;
    constexpr uint8_t reserved2 = 0x04;

    struct MemoryBlockHeader {
        uint64_t magic;

        size_t size;
        uint8_t flags;

        MemoryBlockHeader* next;
        MemoryBlockHeader* prev;
    };

    class FirstFitAllocator {
    public:
        bool initialize(uintptr_t heapStartAddr, uintptr_t heapEndAddr, size_t minBlockSize = 32, size_t heapAlignment = 16);

        uint8_t* allocate(size_t size);
        void deallocate(uint8_t* ptr);

    private:
        MemoryBlockHeader* split(MemoryBlockHeader* mbh, size_t size);
        void merge(MemoryBlockHeader* mbh1, MemoryBlockHeader* mbh2);

        uintptr_t alignAddress(uintptr_t addr);

        uintptr_t dataStart(MemoryBlockHeader* mbh);
        uintptr_t dataEnd(MemoryBlockHeader* mbh);

        uintptr_t initMemory(uintptr_t heapStartAddr, uintptr_t heapEndAddr);

        uint8_t* getData(MemoryBlockHeader* mbh);

        MemoryBlockHeader* getHeader(uint8_t* ptr);

        size_t usableHeapSize;
        size_t minBlockSize;
        size_t heapAlignment;

        MemoryBlockHeader* firstMemoryBlock;
        MemoryBlockHeader* lastMemoryBlock;
    };

    static_assert(std::is_trivially_constructible_v<MemoryBlockHeader>);
    static_assert(std::is_trivially_destructible_v<MemoryBlockHeader>);

    static_assert(std::is_trivially_constructible_v<FirstFitAllocator>);
    static_assert(std::is_trivially_destructible_v<FirstFitAllocator>);
}