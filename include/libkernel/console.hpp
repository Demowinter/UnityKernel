#pragma once
#include <string_view>
#include <cstddef>
#include <cstdint>

namespace Kernel::Console {
    struct Framebuffer {
        uintptr_t address;
        uint32_t pitch;
        uint32_t width;
        uint32_t height;
        uint8_t bpp;
        uint8_t type;
        uint8_t redPosition;
        uint8_t redMaskSize;
        uint8_t greenPosition;
        uint8_t greenMaskSize;
        uint8_t bluePosition;
        uint8_t blueMaskSize;
    };

    bool initialize(const Framebuffer& framebuffer);
    bool isReady();

    void clear();
    void newline();

    void write(std::byte byte);
    void write(const void* str, uint32_t size, uint8_t color = 0x07);

    void print(std::string_view str, uint8_t color = 0x07);
    void println(std::string_view str, uint8_t color = 0x07);
}
