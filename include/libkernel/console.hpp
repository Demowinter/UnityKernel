#pragma once
#include <string_view>
#include <cstddef>
#include <cstdint>

namespace Kernel::Console {
    constexpr uint32_t rows = 25;
    constexpr uint32_t columns = 80;
    constexpr uint32_t screenSize = rows * columns;

    void clear();
    void newline();

    void write(std::byte byte, uint8_t color);
    void write(const void* str, size_t size, uint8_t color = 0x07);

    void print(std::string_view str, uint8_t color = 0x07);
    void print(int64_t number, uint8_t color = 0x07);

    void println(std::string_view str, uint8_t color = 0x07);

    void ok(std::string_view str, bool newline = true);
    void info(std::string_view str, bool newline = true);
    void warn(std::string_view str, bool newline = true);
    void fail(std::string_view str, bool newline = true);
}