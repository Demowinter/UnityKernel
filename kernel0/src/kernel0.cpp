#include <array>
#include <algorithm>
#include <string_view>
#include <variant>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <cpu.hpp>
#include <console.hpp>
#include <memory.hpp>

namespace Kernel {
    constexpr uint32_t multibootMagic = 0x36D76289;

    extern "C" void kernel0(uint32_t mbMagic, uint32_t mbAddr) {
        Console::clear();

        if (mbMagic != multibootMagic) Console::println("Multiboot structure is corrupted");
        else Console::println("Multiboot structure is OK");

        CPU::Info info = CPU::cpuid(0);

        Console::print("CPU manufacturer: ");
        Console::write(&info.ebx, sizeof(info) - sizeof(info.eax), 5);
        Console::newline();

        Console::println("Started kernel0");
        Console::print("Welcome to "); 
        Console::println("UnityKernel v0.1.0-alpha", 5);
        //memmove test
        Console::println("Testing memmove...", 2);

        char moveRight[] = "ABCDE";
        memmove(moveRight + 1, moveRight, 4);
        Console::print("Right: ");
        Console::println(moveRight, 5);

        char moveLeft[] = "ABCDE";
        memmove(moveLeft, moveLeft + 1, 4);
        Console::print("Left:  ");
        Console::println(moveLeft, 5);

        char source[] = "HELLO";
        char destination[6] = {};
        memmove(destination, source, sizeof(source));
        Console::print("Copy:  ");
        Console::println(destination, 5);
    }
}
