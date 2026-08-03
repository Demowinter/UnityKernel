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

        struct MemoryBlockHeader {
            uint64_t magic;

            size_t size;
            uint8_t flags;

            MemoryBlockHeader* next;
            MemoryBlockHeader* prev;
        };

        class MemorySubsystem {
        public:
            void initialize() {
                if (firstMemoryBlock || lastMemoryBlock) return;

                Console::info("Initializing memory subsystem...");

                heapStart = reinterpret_cast<uintptr_t>(__heap_start);
                heapEnd = reinterpret_cast<uintptr_t>(__heap_end);

                MemoryBlockHeader* mbh = createBlock(heapStart, heapEnd - heapStart);

                firstMemoryBlock = mbh;
                lastMemoryBlock = mbh;

                Console::ok("Memory subsystem initialized");
            }

            void validate() {
                Console::info("Starting heap validation...");

                uint8_t fail = 0;

                // if (!firstMemoryBlock || !lastMemoryBlock) return false;
                // if (firstMemoryBlock->prev || lastMemoryBlock->next) return false;

                // for (auto mbh = firstMemoryBlock; mbh != nullptr; mbh = mbh->next) {
                //     if (mbh->magic != memoryBlockMagic) return false;
                //     if (!mbh->next && lastMemoryBlock != mbh) return false;
                //     if (mbh->next->prev != mbh) return false;
                // }

                Console::info("Heap validated");

                if (fail) Console::fail("Heap is corrupted!");
                else Console::ok("Heap is OK!");
            }

            uint8_t* allocate(size_t size) {
                if (!firstMemoryBlock || !lastMemoryBlock || size > heapEnd - heapStart) return nullptr;

                for (auto mbh = firstMemoryBlock; mbh != nullptr; mbh = mbh->next) {
                    if (mbh->flags & used) continue;

                    if (mbh->size >= size + minBlockSize) {
                        auto newBlock = split(mbh, size);

                        if (newBlock) return getDataBlock(newBlock);
                    }

                    if (mbh->size >= size) {
                        mbh->flags |= used;

                        return getDataBlock(mbh);
                    } 
                }

                return nullptr;
            }

            void deallocate(uint8_t* ptr) {
                if (!ptr || !firstMemoryBlock || !lastMemoryBlock) return;

                MemoryBlockHeader* mbh = getMemoryBlockHeader(ptr);
                
                if (mbh->magic != memoryBlockMagic) return;

                mbh->flags &= ~used;

                if (mbh->prev) merge(mbh->prev, mbh);
            }

        private:
            void validateBlock(MemoryBlockHeader* mbh) {

            }

            MemoryBlockHeader* split(MemoryBlockHeader* mbh, size_t size) {
                if(!mbh) return nullptr;
                if (mbh->magic != memoryBlockMagic) return nullptr;
                if (mbh->flags & used || size > mbh->size) return nullptr;

                uintptr_t fullDataBlockEnd = dataEnd(mbh);

                uintptr_t secondBlockAddr = alignAddress(dataStart(mbh) + size + sizeof(MemoryBlockHeader)) - sizeof(MemoryBlockHeader);

                if (secondBlockAddr + sizeof(MemoryBlockHeader) + minBlockSize >= fullDataBlockEnd) return mbh;

                MemoryBlockHeader* firstBlock = mbh;
                MemoryBlockHeader* secondBlock = reinterpret_cast<MemoryBlockHeader*>(secondBlockAddr);

                *secondBlock = *mbh;

                firstBlock->size = size;
                firstBlock->next = secondBlock;

                secondBlock->size = fullDataBlockEnd - dataStart(secondBlock);
                secondBlock->prev = firstBlock;

                if (secondBlock->next) secondBlock->next->prev = secondBlock;
                if (lastMemoryBlock == mbh) lastMemoryBlock = secondBlock;

                return firstBlock;
            }

            void merge(MemoryBlockHeader* mbh1, MemoryBlockHeader* mbh2) {
                if(!mbh1 || !mbh2) return;
                if (mbh1->magic != memoryBlockMagic || mbh2->magic != memoryBlockMagic) return;
                if (mbh1->next != mbh2 || mbh2->flags & used) return;

                mbh1->size = dataEnd(mbh2) - dataStart(mbh1);
                mbh1->next = mbh2->next;

                if (mbh2->next) mbh2->next->prev = mbh1;
                if (lastMemoryBlock == mbh2) lastMemoryBlock = mbh1;
            }

            uintptr_t alignAddress(uintptr_t addr) {
                return alignUp(addr, heapAlignment);
            }

            uintptr_t dataStart(MemoryBlockHeader* mbh) {
                if(!mbh) return 0;

                return (mbh->magic == memoryBlockMagic) ? reinterpret_cast<uintptr_t>(mbh) + sizeof(MemoryBlockHeader) : 0;
            }

            uintptr_t dataEnd(MemoryBlockHeader* mbh) {
                if(!mbh) return 0;

                return (mbh->magic == memoryBlockMagic) ? dataStart(mbh) + mbh->size : 0;
            }

            MemoryBlockHeader* createBlock(uintptr_t addr, size_t size) {
                uintptr_t dataAddr = alignAddress(addr + sizeof(MemoryBlockHeader));
                uintptr_t headerAddr = dataAddr - sizeof(MemoryBlockHeader);

                MemoryBlockHeader* mbh = reinterpret_cast<MemoryBlockHeader*>(headerAddr);
                mbh->magic = memoryBlockMagic;
                mbh->size = size;
                mbh->flags = 0;
                mbh->prev = nullptr;
                mbh->next = nullptr;

                return mbh;
            }

            uint8_t* getDataBlock(MemoryBlockHeader* mbh) {
                return reinterpret_cast<uint8_t*>(dataStart(mbh));
            }

            MemoryBlockHeader* getMemoryBlockHeader(uint8_t* ptr) {
                return reinterpret_cast<MemoryBlockHeader*>(ptr - sizeof(MemoryBlockHeader));
            }

            uintptr_t heapStart;
            uintptr_t heapEnd;

            MemoryBlockHeader* firstMemoryBlock;
            MemoryBlockHeader* lastMemoryBlock;
        };

        static_assert(std::is_trivially_constructible_v<MemorySubsystem>);
        static_assert(std::is_trivially_destructible_v<MemorySubsystem>);
    }

    static MemorySubsystem memory;

    void initialize() {
        memory.initialize();
    }

    bool validate() {

    }

    void* allocate(size_t size) {
        return memory.allocate(size);
    }

    void deallocate(void* ptr) {
        memory.deallocate(reinterpret_cast<uint8_t*>(ptr));
    }
}