#include <memory>
#include <type_traits>
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

                uintptr_t heapStartAddr = reinterpret_cast<uintptr_t>(__heap_start);
                uintptr_t heapEndAddr = reinterpret_cast<uintptr_t>(__heap_end);

                uintptr_t heapDataAddr = initMemory(heapStartAddr, heapEndAddr);

                if (heapDataAddr) {
                    Console::ok("Memory subsystem initialized");
                    Console::info("Heap starts on: 0x", false);

                    std::unique_ptr<char[]> buffer{new char[16]};
                    Console::println(ltoa(heapDataAddr, buffer.get(), 16));
                }

                else Console::fail("Memory subsystem initialization failed");
            }

            // void validate() {
            //     Console::info("Starting heap validation...");

            //     uint8_t ok = 0;

            //     // if (!firstMemoryBlock || !lastMemoryBlock) return false;
            //     // if (firstMemoryBlock->prev || lastMemoryBlock->next) return false;

            //     // for (auto mbh = firstMemoryBlock; mbh != nullptr; mbh = mbh->next) {
            //     //     if (mbh->magic != memoryBlockMagic) return false;
            //     //     if (!mbh->next && lastMemoryBlock != mbh) return false;
            //     //     if (mbh->next->prev != mbh) return false;
            //     // }

            //     Console::info("Heap validated");

            //     if (ok) Console::ok("Heap is OK!");
            //     else Console::fail("Heap is corrupted!");
            // }

            uint8_t* allocate(size_t size) {
                if (!firstMemoryBlock || !lastMemoryBlock) return nullptr;
                if (!size || size > usableHeapSize) return nullptr;

                for (auto mbh = firstMemoryBlock; mbh != nullptr; mbh = mbh->next) {
                    if (mbh->flags & used) continue;

                    if (mbh->size >= size + minBlockSize) {
                        auto newBlock = split(mbh, size);

                        if (newBlock) {
                            newBlock->flags |= used;

                            return getData(newBlock);
                        }
                    }

                    if (mbh->size >= size) {
                        mbh->flags |= used;

                        return getData(mbh);
                    } 
                }

                return nullptr;
            }

            void deallocate(uint8_t* ptr) {
                if (!ptr || !firstMemoryBlock || !lastMemoryBlock) return;

                MemoryBlockHeader* mbh = getHeader(ptr);
                
                if (mbh->magic != memoryBlockMagic) return;
                if (!(mbh->flags & used)) return;

                mbh->flags &= ~used;

                if (mbh->next && !(mbh->next->flags & used)) merge(mbh, mbh->next);
                if (mbh->prev && !(mbh->prev->flags & used)) merge(mbh->prev, mbh);
            }

        private:
            // void validateBlock(MemoryBlockHeader* mbh) {

            // }

            MemoryBlockHeader* split(MemoryBlockHeader* mbh, size_t size) {
                if(!mbh) return nullptr;
                if (mbh->magic != memoryBlockMagic) return nullptr;
                if (mbh->flags & used || size > mbh->size) return nullptr;

                uintptr_t fullDataStartAddr = dataStart(mbh);
                uintptr_t fullDataEndAddr = dataEnd(mbh);

                uintptr_t secondDataAddr = alignAddress(fullDataStartAddr + size + sizeof(MemoryBlockHeader));
                uintptr_t secondHeaderAddr = secondDataAddr - sizeof(MemoryBlockHeader);

                if (secondDataAddr + minBlockSize >= fullDataEndAddr) return mbh;

                MemoryBlockHeader* firstBlock = mbh;
                MemoryBlockHeader* secondBlock = reinterpret_cast<MemoryBlockHeader*>(secondHeaderAddr);

                *secondBlock = *mbh;

                firstBlock->size = size;
                firstBlock->next = secondBlock;

                secondBlock->size = fullDataEndAddr - secondDataAddr;
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

            uintptr_t initMemory(uintptr_t heapStartAddr, uintptr_t heapEndAddr) {
                uintptr_t dataAddr = alignAddress(heapStartAddr + sizeof(MemoryBlockHeader));
                uintptr_t headerAddr = dataAddr - sizeof(MemoryBlockHeader);

                usableHeapSize = heapEndAddr - dataAddr;

                if (dataAddr + minBlockSize >= heapEndAddr) return 0;

                MemoryBlockHeader* mbh = reinterpret_cast<MemoryBlockHeader*>(headerAddr);
                mbh->magic = memoryBlockMagic;
                mbh->size = usableHeapSize;
                mbh->flags = 0;
                mbh->prev = nullptr;
                mbh->next = nullptr;

                firstMemoryBlock = mbh;
                lastMemoryBlock = mbh;

                return dataAddr;
            }

            uint8_t* getData(MemoryBlockHeader* mbh) {
                return reinterpret_cast<uint8_t*>(dataStart(mbh));
            }

            MemoryBlockHeader* getHeader(uint8_t* ptr) {
                return reinterpret_cast<MemoryBlockHeader*>(ptr - sizeof(MemoryBlockHeader));
            }

            size_t usableHeapSize;

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

    // bool validate() {

    // }

    void* allocate(size_t size) {
        return memory.allocate(size);
    }

    void deallocate(void* ptr) {
        memory.deallocate(static_cast<uint8_t*>(ptr));
    }
}