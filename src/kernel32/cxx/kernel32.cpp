#include <memory>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <libarch/api.hpp>
#include <libkrt/krt.hpp>
#include <libkernel/memory.hpp>
#include <libkernel/console.hpp>
#include <kernel32/grub.hpp>

namespace Kernel {
    extern "C" [[noreturn]] void kernel32(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        Console::info("Starting kernel32...");

        Memory::initialize();
        KernelRT::initialize();

        if (!GRUB::checkMultiboot(mbMagic)) Console::info("Multiboot structure is corrupted");
        else Console::info("Multiboot structure is OK");

        Console::info("CPU manufacturer: ", false);
        Console::println(Arch::CPU::manufacturer(), 0x05);

        Console::ok("Started kernel32");

        Console::newline();

        Console::print("Welcome to ");
        Console::println("UnityKernel! v0.1.0-alpha", 5);

        KernelRT::finalize();

        Arch::CPU::halt();
    }
}