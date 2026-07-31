#include <cstdint>
#include <cstddef>
#include <libkernel/cpu.hpp>
#include <libkernel/console.hpp>

namespace Kernel {
    constexpr uint32_t multibootMagic = 0x36D76289;

    extern "C" void kernel32(uint32_t mbMagic, uint32_t mbAddr) {
        Console::clear();

        if (mbMagic != multibootMagic) Console::println("Multiboot structure is corrupted");
        else Console::println("Multiboot structure is OK");

        CPU::Info info = CPU::cpuid(0);
        auto binaryName = CPU::manufacturer(info);

        Console::print("CPU manufacturer: ");
        Console::write(binaryName.data(), binaryName.size() * sizeof(uint32_t));
        Console::newline();

        Console::println("Started kernel32!");
        
        Console::newline();

        Console::println("Welcome to UnityKernel! v0.1.0-alpha");
    }
}