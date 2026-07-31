#include <cstdint>
#include <cstddef>
#include "cpu.hpp"
#include "console.hpp"

namespace Kernel {
    extern "C" void kernel64(uint64_t mbMagic, uint64_t mbi) {
        Console::clear();

        CPU::Info info;
        CPU::cpuid(0, info);

        // Console::print("CPU manufacturer: ");
        // Console::write(&info.ebx, sizeof(info) - sizeof(info.eax));
        // Console::newline();

        Console::println("Started kernel0");

        // while (true) {}
        

        // Console::newline();

        // std::variant<int, float, std::string_view> v = "Hello, Developer! This is the std::string_view in std::variant!";

        // Console::println("Testing std::variant...");

        // std::visit([](auto&& obj) -> void {
        //     Console::println("Visiting std::variant...");

        //     using T = std::decay_t<decltype(obj)>;

        //     if constexpr (std::is_same_v<T, int>) Console::println("std::variant has type int");
        //     else if constexpr (std::is_same_v<T, float>) Console::println("std::variant has type float");
        //     else if constexpr (std::is_same_v<T, std::string_view>) {
        //         Console::println("std::variant has type std::string_view");

        //         Console::print("Extracted value: ");
        //         Console::print(obj);
        //     }

        // }, v);

        // Console::newline();
        // Console::print("Hello!");
    }
}