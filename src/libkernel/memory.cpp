#include <cstddef>
#include <cstdint>
#include <libkbase/algo.hpp>
#include <libkernel/console.hpp>
#include <libkernel/memory.hpp>

namespace Kernel::Memory {
    namespace {
        extern "C" {
            extern uint8_t __heap_start[];
            extern uint8_t __heap_end[];
        }

        constexpr uint64_t memoryBlockMagic = 0xF5CD3EE914E9D29F;

        constexpr uint8_t used = 0x01;
        constexpr uint8_t reserved1 = 0x02;
        constexpr uint8_t reserved2 = 0x04;

        struct MemoryBlock {
            uint64_t magic;

            size_t size;
            uint8_t flags;

            MemoryBlock* next;
            MemoryBlock* prev;
        };

        class MemorySubsystem {
        public:
            void initialize() {
                if (firstMemoryBlock || lastMemoryBlock) return;

                Console::info("Initializing memory subsystem...");

                heapStart = reinterpret_cast<uintptr_t>(__heap_start);
                heapEnd = reinterpret_cast<uintptr_t>(__heap_end);

                MemoryBlock* mb = createBlock(heapStart, heapEnd - heapStart);

                firstMemoryBlock = mb;
                lastMemoryBlock = mb;

                Console::ok("Memory subsystem initialized");
            }

            uint8_t* allocate(size_t size) {
                if (!firstMemoryBlock || !lastMemoryBlock || size > heapEnd - heapStart) return nullptr;

                for (auto mb = firstMemoryBlock; mb != nullptr; mb = mb->next) {
                    if (mb->flags & used) continue;

                    if (mb->size >= size + minBlockSize) {
                        auto newBlock = split(mb, size);

                        if (newBlock) return getDataBlock(newBlock);
                    }

                    if (mb->size >= size) {
                        mb->flags |= used;

                        return getDataBlock(mb);
                    } 
                }

                return nullptr;
            }

            void deallocate(uint8_t* ptr) {
                if (!ptr || !firstMemoryBlock || !lastMemoryBlock) return;

                MemoryBlock* mb = getMemoryBlock(ptr);
                
                if (mb->magic != memoryBlockMagic) return;

                mb->flags &= ~used;

                if (mb->prev) merge(mb->prev, mb);
            }

        private:
            MemoryBlock* split(MemoryBlock* mb, size_t size) {
                if(!mb) return nullptr;
                if (mb->magic != memoryBlockMagic) return nullptr;
                if (mb->flags & used || size > mb->size) return nullptr;

                uintptr_t fullDataBlockEnd = dataBlockEnd(mb);

                uintptr_t secondBlockAddr = alignAddress(dataBlockStart(mb) + size + sizeof(MemoryBlock)) - sizeof(MemoryBlock);

                if (secondBlockAddr + sizeof(MemoryBlock) + minBlockSize >= fullDataBlockEnd) return mb;

                MemoryBlock* firstBlock = mb;
                MemoryBlock* secondBlock = reinterpret_cast<MemoryBlock*>(secondBlockAddr);

                *secondBlock = *mb;

                firstBlock->size = size;
                firstBlock->next = secondBlock;

                secondBlock->size = fullDataBlockEnd - dataBlockStart(secondBlock);
                secondBlock->prev = firstBlock;

                if (secondBlock->next) secondBlock->next->prev = secondBlock;
                if (lastMemoryBlock == mb) lastMemoryBlock = secondBlock;

                return firstBlock;
            }

            void merge(MemoryBlock* mb1, MemoryBlock* mb2) {
                if(!mb1 || !mb2) return;
                if (mb1->magic != memoryBlockMagic || mb2->magic != memoryBlockMagic) return;
                if (mb1->next != mb2 || mb2->flags & used) return;

                mb1->size = dataBlockEnd(mb2) - dataBlockStart(mb1);
                mb1->next = mb2->next;

                if (mb2->next) mb2->next->prev = mb1;
                if (lastMemoryBlock == mb2) lastMemoryBlock = mb1;
            }

            uintptr_t alignAddress(uintptr_t addr) {
                uintptr_t blockAddrAligned = alignUp(addr + sizeof(MemoryBlock), heapAlignment);
                uintptr_t blockAddr = blockAddrAligned - sizeof(MemoryBlock);

                return blockAddr;
            }

            uintptr_t dataBlockStart(MemoryBlock* mb) {
                if(!mb) return 0;

                return (mb->magic == memoryBlockMagic) ? reinterpret_cast<uintptr_t>(mb) + sizeof(MemoryBlock) : 0;
            }

            uintptr_t dataBlockEnd(MemoryBlock* mb) {
                if(!mb) return 0;

                return (mb->magic == memoryBlockMagic) ? dataBlockStart(mb) + mb->size : 0;
            }

            MemoryBlock* createBlock(uintptr_t addr, size_t size) {
                uintptr_t blockAddr = alignAddress(addr);

                MemoryBlock* mb = reinterpret_cast<MemoryBlock*>(blockAddr);
                mb->magic = memoryBlockMagic;
                mb->size = size;
                mb->flags = 0;
                mb->prev = nullptr;
                mb->next = nullptr;

                return mb;
            }

            uint8_t* getDataBlock(MemoryBlock* mb) {
                return reinterpret_cast<uint8_t*>(dataBlockStart(mb));
            }

            MemoryBlock* getMemoryBlock(uint8_t* ptr) {
                return reinterpret_cast<MemoryBlock*>(ptr - sizeof(MemoryBlock));
            }

            uintptr_t heapStart;
            uintptr_t heapEnd;

            MemoryBlock* firstMemoryBlock;
            MemoryBlock* lastMemoryBlock;
        };

        static_assert(std::is_trivially_constructible_v<MemorySubsystem>);
        static_assert(std::is_trivially_destructible_v<MemorySubsystem>);
    }

    static MemorySubsystem memory;

    void initialize() {
        memory.initialize();
    }

    void* allocate(size_t size) {
        Console::println("Kernel::Memory::allocate: allocating memory...");

        return memory.allocate(size);
    }

    void deallocate(void* ptr) {
        Console::println("Kernel::Memory::deallocate: deallocating memory...");

        memory.deallocate(reinterpret_cast<uint8_t*>(ptr));
    }
}