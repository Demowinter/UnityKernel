#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <liblltools/allocator.hpp>
#include <kernel/console.hpp>
#include <kernel/memory.hpp>

namespace Kernel::Memory {
    class MemorySubsystem {
    public:
        void initialize(uintptr_t heapStartAddr, uintptr_t heapEndAddr) {
            basicAllocator.initialize(heapStartAddr, heapEndAddr, minBlockSize, heapAlignment);
        }

        uint8_t* allocateBasicHeap(size_t size) {
            return basicAllocator.allocate(size);
        }

        void deallocateBasicHeap(uint8_t* ptr) {
            basicAllocator.deallocate(ptr);
        }
    
    private:
        LLTools::FirstFitAllocator basicAllocator;
    };

    static_assert(std::is_trivially_constructible_v<MemorySubsystem>);
    static_assert(std::is_trivially_destructible_v<MemorySubsystem>);

    static MemorySubsystem memory;

    void initialize(uintptr_t heapStartAddr, uintptr_t heapEndAddr) {
        memory.initialize(heapStartAddr, heapEndAddr);
    }

    void* allocateBasic(size_t size) {
        return memory.allocateBasicHeap(size);
    }

    void deallocateBasic(void* ptr) {
        memory.deallocateBasicHeap(static_cast<uint8_t*>(ptr));
    }
}