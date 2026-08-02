#include <cstddef>
#include <cstdint>
#include <libkbase/math.hpp>
#include <libkernel/memory.hpp>
#include <libkernel/console.hpp>

namespace Kernel::Memory {
    namespace {
        extern "C" {
            extern uint8_t __heap_start[];
            extern uint8_t __heap_end[];
        }

        constexpr uint64_t memoryBlockMagic = 0xF5CD3EE914E9D29F;

        struct MemoryBlock {
            uint64_t magic;

            size_t size;
            uint8_t used;

            MemoryBlock* next;
            MemoryBlock* prev;
        };

        class MemorySubsystem {
        public:
            void initialize() {
                if (firstBlock || lastBlock) return;

                Console::info("Initializing memory subsystem...");

                heapStart = reinterpret_cast<uintptr_t>(__heap_start);
                heapEnd = reinterpret_cast<uintptr_t>(__heap_end);

                MemoryBlock* mb = createBlock(heapStart, heapEnd - heapStart);

                firstBlock = mb;
                lastBlock = mb;

                Console::ok("Memory subsystem initialized");
            }

            uint8_t* allocate(size_t size) {
                if (!firstBlock || !lastBlock || size > heapEnd - heapStart) return nullptr;

                return nullptr;
            }

            void deallocate(uint8_t* ptr) {
                if (!ptr || !firstBlock || !lastBlock) return;

                [[maybe_unused]]
                MemoryBlock* mb = reinterpret_cast<MemoryBlock*>(ptr - sizeof(MemoryBlock));
            }

        private:
            uint8_t* blockStart(MemoryBlock* mb) {
                return reinterpret_cast<uint8_t*>(mb) + sizeof(MemoryBlock);
            }

            uint8_t* blockEnd(MemoryBlock* mb) {
                return blockStart(mb) + mb->size;
            }

            MemoryBlock* createBlock(uintptr_t addr, size_t size) {
                uintptr_t blockAddrAligned = alignUp(addr + sizeof(MemoryBlock), heapAlignment);
                uintptr_t blockAddr = blockAddrAligned - sizeof(MemoryBlock);

                MemoryBlock* mb = reinterpret_cast<MemoryBlock*>(blockAddr);
                mb->magic = memoryBlockMagic;
                mb->size = size;
                mb->used = 0;
                mb->prev = nullptr;
                mb->next = nullptr;

                return mb;
            }

            uintptr_t heapStart;
            uintptr_t heapEnd;

            MemoryBlock* firstBlock;
            MemoryBlock* lastBlock;
        };

        static_assert(std::is_trivially_constructible_v<MemorySubsystem>);
        static_assert(std::is_trivially_destructible_v<MemorySubsystem>);
    }

    static MemorySubsystem memory;

    void initialize() {
        memory.initialize();
    }

    void* allocate(size_t size) {
        Console::println("Kernel::Memory::allocate not implemented yet!");

        return memory.allocate(size);
    }

    void deallocate(void* ptr) {
        Console::println("Kernel::Memory::deallocate not implemented yet!");

        memory.deallocate(reinterpret_cast<uint8_t*>(ptr));
    }
}