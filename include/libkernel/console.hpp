#pragma once
#include <string_view>
#include <cstddef>
#include <cstdint>

namespace Kernel::Console {
    constexpr uint32_t rows = 32;
    constexpr uint32_t columns = 80;
    constexpr uint32_t screenSize = rows * columns;

    void clear();
    void newline();

    void write(std::byte byte);
    void write(const void* str, uint32_t size, uint8_t color = 0x07);

    void print(std::string_view str, uint8_t color = 0x07);
    void println(std::string_view str, uint8_t color = 0x07);
}