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

        Console::newline();

        std::variant<int, float, std::string_view> v = "Hello, Developer! This is the std::string_view in std::variant!";

        Console::println("Testing std::variant...");

        std::visit([](auto&& obj) -> void {
            Console::println("Visiting std::variant...");

            using T = std::decay_t<decltype(obj)>;

            if constexpr (std::is_same_v<T, int>) Console::println("std::variant has type int");
            else if constexpr (std::is_same_v<T, float>) Console::println("std::variant has type float");
            else if constexpr (std::is_same_v<T, std::string_view>) {
                Console::println("std::variant has type std::string_view");

                Console::print("Extracted value: ");
                Console::print(obj);
            }

        }, v);

        std::string_view str2 = std::get<std::string_view>(v);

        Console::newline();
        Console::print("Hello!");
    }
}