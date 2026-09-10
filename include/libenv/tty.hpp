#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ENV::TTY {
    void clear();
    void newline();

    void write(std::byte byte, uint8_t color);
    void write(const void* str, size_t size, uint8_t color = 0x07);

    void print(std::string_view str, uint8_t color = 0x07);
    void println(std::string_view str, uint8_t color = 0x07);

    void ok(std::string_view str, bool newline = true);
    void info(std::string_view str, bool newline = true);
    void warn(std::string_view str, bool newline = true);
    void fail(std::string_view str, bool newline = true);

    //Called by keyboard interrupt handler
    void onKeyPress(uint8_t scancode, bool shift, bool ctrl, bool alt);
}