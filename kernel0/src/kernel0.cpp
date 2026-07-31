#include <array>
#include <algorithm>
#include <string_view>
#include <variant>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <cpu.hpp>
#include <console.hpp>

namespace Kernel {
    constexpr uint32_t multibootMagic = 0x36D76289;

    extern "C" void kernel0(uint32_t mbMagic, uint32_t mbAddr) {
        Console::clear();

        if (mbMagic != multibootMagic) Console::println("Multiboot structure is corrupted");
        else Console::println("Multiboot structure is OK");

        CPU::Info info = CPU::cpuid(0);

        Console::print("CPU manufacturer: ");
        Console::write(&info.ebx, sizeof(info) - sizeof(info.eax));
        Console::newline();

        Console::println("Started kernel0");
        Console::println("Welcome to UnityKernel! v0.1.0-alpha");
    }
}