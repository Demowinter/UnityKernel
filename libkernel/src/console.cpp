#include <string_view>
#include <cstddef>
#include <cstdint>
#include <console.hpp>

namespace Kernel::Console {
    static uint16_t* video = reinterpret_cast<uint16_t*>(0xB8000);
    static uint16_t cursor = 0;

    void clear() {
        for (uint32_t i = 0; i < screenSize; i++) video[i] = 0x00;

        cursor = 0;
    }

    void newline() {
        cursor += columns - (cursor  % columns);
    }

    void write(std::byte byte) {
        video[cursor++] = (0x07 << 8) | static_cast<uint8_t>(byte);
    }

    void write(const void* str, uint32_t size, uint8_t color) {
        for (uint32_t i = 0; i < size; i++) video[cursor++] = (color << 8) | reinterpret_cast<const uint8_t*>(str)[i];
    }

    void print(std::string_view str, uint8_t color) {
        for (char ch : str) video[cursor++] = (color << 8) | ch;
    }

    void println(std::string_view str, uint8_t color) {
        print(str, color);

        newline();
    }
}