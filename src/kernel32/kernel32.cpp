#include <memory>
#include <cstdint>
#include <cstddef>
#include <libkrt/krt.hpp>
#include <libkernel/cpu.hpp>
#include <libkernel/memory.hpp>
#include <libkernel/console.hpp>
#include <kernel32/grub.hpp>

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

        CPU::interrupts(false);
        CPU::halt();
    }
}