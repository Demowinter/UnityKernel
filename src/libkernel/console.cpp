#include <string_view>
#include <cstddef>
#include <cstdint>
#include <libkernel/console.hpp>

namespace Kernel::Console {
    static uint16_t* video = reinterpret_cast<uint16_t*>(0xB8000);
    static uint16_t cursor = 0;

    void scroll() {
        for (size_t i = columns; i < screenSize; i++) video[i - columns] = video[i];
        for (size_t i = screenSize - columns; i < screenSize; i++) video[i] = 0x00;
        cursor -= columns;
    }

    void clear() {
        for (size_t i = 0; i < screenSize; i++) video[i] = 0x00;

        cursor = 0;
    }

    void newline() {
        if (cursor >= columns * rows) {
            while (cursor >= screenSize) scroll();
        }

        cursor += columns - (cursor % columns);
        if (cursor >= screenSize) {
            while (cursor >= screenSize) scroll();
        }
    }

    void write(std::byte byte, uint8_t color) {
        if (cursor >= screenSize) {
            while (cursor >= screenSize) scroll();
        }

        video[cursor++] = (color << 8) | static_cast<uint8_t>(byte);
    }

    void write(const void* str, size_t size, uint8_t color) {
        for (size_t i = 0; i < size; i++) write(std::byte{reinterpret_cast<const uint8_t*>(str)[i]}, color);
    }

    void print(std::string_view str, uint8_t color) {
        for (char ch : str) write(std::byte{static_cast<uint8_t>(ch)}, color);
    }

    void println(std::string_view str, uint8_t color) {
        print(str, color);

        newline();
    }

    void ok(std::string_view str, bool newline) {
        print("[  ");
        print("OK", 0x0A);
        print("  ] ");

        print(str);

        if (newline) Console::newline();
    }

    void info(std::string_view str, bool newline) {
        print("[ ");
        print("INFO", 0x0F);
        print(" ] ");

        print(str);

        if (newline) Console::newline();
    }

    void warn(std::string_view str, bool newline) {
        print("[ ");
        print("WARN", 0x0E);
        print(" ] ");

        print(str);

        if (newline) Console::newline();
    }

    void fail(std::string_view str, bool newline) {
        print("[");
        print("FAILED", 0x0C);
        print("] ");

        print(str);

        if (newline) Console::newline();
    }
}