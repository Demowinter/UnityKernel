#include <cstdint>
#include <cstddef>
#include <libkrt/krt.hpp>
#include <libkernel/cpu.hpp>
#include <libkernel/memory.hpp>
#include <libkernel/console.hpp>
#include <libkstd/assert.hpp>
#include <kernel32/grub.hpp>

namespace Kernel::Tests
{
    void testBasicAllocation()
    {
        void* ptr = Kernel::Memory::allocate(64);

        KernelSTD::assert(
            ptr != nullptr,
            "Basic allocation"
        );

        Kernel::Memory::deallocate(ptr);
    }


    void testMultipleAllocation()
    {
        void* a = Kernel::Memory::allocate(64);
        void* b = Kernel::Memory::allocate(128);
        void* c = Kernel::Memory::allocate(256);

        KernelSTD::assert(
            a != nullptr && b != nullptr && c != nullptr,
            "Multiple allocation"
        );

        KernelSTD::assert(
            a != b && a != c && b != c,
            "Unique addresses"
        );

        Kernel::Memory::deallocate(a);
        Kernel::Memory::deallocate(b);
        Kernel::Memory::deallocate(c);
    }


    void testWriteRead()
    {
        constexpr size_t size = 256;

        uint8_t* ptr =
            static_cast<uint8_t*>(Kernel::Memory::allocate(size));

        bool valid = ptr != nullptr;

        if (valid)
        {
            for (size_t i = 0; i < size; i++)
                ptr[i] = static_cast<uint8_t>(i);

            for (size_t i = 0; i < size; i++)
            {
                if (ptr[i] != static_cast<uint8_t>(i))
                {
                    valid = false;
                    break;
                }
            }
        }

        KernelSTD::assert(
            valid,
            "Memory write/read"
        );

        Kernel::Memory::deallocate(ptr);
    }


    void testAlignment()
    {
        constexpr size_t alignment = 16;

        bool valid = true;

        for (size_t size = 1; size <= 128; size++)
        {
            uintptr_t ptr =
                reinterpret_cast<uintptr_t>(
                    Kernel::Memory::allocate(size)
                );

            if (!ptr || ptr % alignment != 0)
            {
                valid = false;
                break;
            }

            Kernel::Memory::deallocate(
                reinterpret_cast<void*>(ptr)
            );
        }

        KernelSTD::assert(
            valid,
            "Memory alignment"
        );
    }


    void testSplit()
    {
        void* block = Kernel::Memory::allocate(512);

        Kernel::Memory::deallocate(block);

        void* small = Kernel::Memory::allocate(64);

        KernelSTD::assert(
            small == block,
            "Block split"
        );

        Kernel::Memory::deallocate(small);
    }


    void testMerge()
    {
        void* a = Kernel::Memory::allocate(128);
        void* b = Kernel::Memory::allocate(128);

        Kernel::Memory::deallocate(a);
        Kernel::Memory::deallocate(b);

        void* merged = Kernel::Memory::allocate(256);

        KernelSTD::assert(
            merged == a,
            "Block merge"
        );

        Kernel::Memory::deallocate(merged);
    }


    void testFreeOrder()
    {
        void* blocks[8];

        for (size_t i = 0; i < 8; i++)
            blocks[i] = Kernel::Memory::allocate(64);

        for (size_t i = 0; i < 8; i += 2)
            Kernel::Memory::deallocate(blocks[i]);

        for (size_t i = 1; i < 8; i += 2)
            Kernel::Memory::deallocate(blocks[i]);

        void* block = Kernel::Memory::allocate(512);

        KernelSTD::assert(
            block != nullptr,
            "Free order"
        );

        Kernel::Memory::deallocate(block);
    }


    void testFullHeap()
    {
        constexpr size_t blockSize = 4096;
        constexpr size_t maxBlocks = 256;

        void* blocks[maxBlocks];

        size_t count = 0;

        while (count < maxBlocks)
        {
            void* ptr = Kernel::Memory::allocate(blockSize);

            if (!ptr)
                break;

            blocks[count++] = ptr;
        }

        bool valid = count > 0;

        for (size_t i = 0; i < count; i++)
            Kernel::Memory::deallocate(blocks[i]);

        KernelSTD::assert(
            valid,
            "Heap exhaustion"
        );
    }


    void testStress()
    {
        constexpr size_t count = 128;

        void* blocks[count]{};

        bool valid = true;

        for (size_t i = 0; i < 10000; i++)
        {
            size_t index = i % count;

            if (blocks[index])
            {
                Kernel::Memory::deallocate(blocks[index]);
                blocks[index] = nullptr;
            }

            blocks[index] =
                Kernel::Memory::allocate((i % 512) + 1);

            if (!blocks[index])
            {
                valid = false;
                break;
            }
        }

        for (size_t i = 0; i < count; i++)
        {
            if (blocks[i])
                Kernel::Memory::deallocate(blocks[i]);
        }

        KernelSTD::assert(
            valid,
            "Memory stress"
        );
    }


    void runMemoryTests()
    {
        Console::info("Starting memory testing...");

        testBasicAllocation();
        testMultipleAllocation();
        testWriteRead();
        testAlignment();
        testSplit();
        testMerge();
        testFreeOrder();
        testFullHeap();
        testStress();
    }
}

namespace Kernel {
    void kassert(bool condition, std::string_view) {

    }

    extern "C" [[noreturn]] void kernel32(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        Console::clear();
        Console::info("Starting kernel32...");

        Memory::initialize();
        KernelRT::initialize();

        if (!GRUB::checkMultiboot(mbMagic)) Console::info("Multiboot structure is corrupted");
        else Console::info("Multiboot structure is OK");

        CPU::Info info = CPU::cpuid(0);
        auto name = CPU::manufacturer(info);

        Console::info("CPU manufacturer: ", false);
        Console::write(name, sizeof(info) - sizeof(info.eax), 5);
        Console::newline();

        Console::ok("Started kernel32");
        delete[] name;

        Console::newline();

        Console::print("Welcome to ");
        Console::println("UnityKernel! v0.1.0-alpha", 5);

        Console::newline();

        Console::info("Testing log messages:");
        Console::newline();

        Console::ok("Ok");
        Console::info("Information");
        Console::warn("Warning");
        Console::fail("Failed");

        Console::newline();

        // Console::info("Test number print: ", false);
        // Console::print(42);

        // int i = 0;

        // while (true) {
        //     Console::print(i++, 0x02);
        //     Console::newline();
        // }

        Tests::runMemoryTests();


        CPU::interrupts(false);
        CPU::halt();
    }
}