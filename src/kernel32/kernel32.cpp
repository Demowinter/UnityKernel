#include <cstdint>
#include <cstddef>
#include <libkernel/cpu.hpp>
#include <libkernel/console.hpp>
#include <kernel32/grub.hpp>

namespace Kernel {    
    extern "C" [[noreturn]] void kernel32(uint32_t mbMagic, uint32_t mbAddr) {
        Console::clear();

        if (!GRUB::checkMultiboot(mbMagic)) Console::println("Multiboot structure is corrupted");
        else Console::println("Multiboot structure is OK");

        CPU::Info info = CPU::cpuid(0);
        auto binaryName = CPU::manufacturer(info);

        Console::print("CPU manufacturer: ");
        Console::write(binaryName.data(), binaryName.size() * sizeof(uint32_t), 5);
        Console::newline();

        Console::println("Started kernel32!");
        
        Console::newline();

        Console::print("Welcome to ");
        Console::print("UnityKernel! v0.1.0-alpha", 5);

        CPU::interrupts(false);
        CPU::halt();
    }
}