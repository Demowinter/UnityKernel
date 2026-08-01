#pragma once
#include <cstdint>

namespace Kernel::GRUB {
    constexpr uint32_t multibootMagic = 0x36D76289;

    struct [[gnu::packed]] Framebuffer {
        uint32_t type;
        uint32_t size;
        uint64_t address;
        uint32_t pitch;
        uint32_t width;
        uint32_t height;
        uint8_t bpp;
        uint8_t framebufferType;
        uint16_t reserved;
        uint8_t redPosition;
        uint8_t redMaskSize;
        uint8_t greenPosition;
        uint8_t greenMaskSize;
        uint8_t bluePosition;
        uint8_t blueMaskSize;
    };

    bool checkMultiboot(uint32_t magic);
    const Framebuffer* findFramebuffer(uint32_t multibootInfoAddress);
}
