#include <string_view>
#include <cstddef>
#include <cstdint>
#include <libkernel/console.hpp>

namespace Kernel::Console {
    static uint16_t* video = reinterpret_cast<uint16_t*>(0xB8000);
    static uint16_t cursor = 0;

    void clear() {
        for (size_t i = 0; i < screenSize; i++) video[i] = 0x00;

        cursor = 0;
    }

    void newline() {
        if (cursor >= columns * rows) {
            for (size_t i = columns; i < cursor + columns; i++) video[i - columns] = video[i];

            cursor -= columns;
        }

        cursor += columns - (cursor % columns);
    }

    void write(std::byte byte, uint8_t color) {
        video[cursor++] = (color << 8) | static_cast<uint8_t>(byte);
    }

    void write(const void* str, size_t size, uint8_t color) {
        for (size_t i = 0; i < size; i++) video[cursor++] = (color << 8) | reinterpret_cast<const uint8_t*>(str)[i];
    }

    void print(std::string_view str, uint8_t color) {
        for (char ch : str) video[cursor++] = (color << 8) | ch;
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