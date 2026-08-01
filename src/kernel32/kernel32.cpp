#include <cstdint>
#include <cstddef>
#include <libkernel/cpu.hpp>
#include <libkernel/console.hpp>
#include <kernel32/grub.hpp>

namespace Kernel {    
    uint32_t makeColor(const GRUB::Framebuffer& fb,
                       uint8_t red, uint8_t green, uint8_t blue) {
        return (uint32_t(red >> (8 - fb.redMaskSize)) << fb.redPosition) |
               (uint32_t(green >> (8 - fb.greenMaskSize)) << fb.greenPosition) |
               (uint32_t(blue >> (8 - fb.blueMaskSize)) << fb.bluePosition);
    }

    void putPixel(const GRUB::Framebuffer& fb, uint32_t x, uint32_t y,
                  uint8_t red, uint8_t green, uint8_t blue) {
        if (x >= fb.width || y >= fb.height || fb.bpp != 32 || fb.framebufferType != 1) {
            return;
        }

        auto* row = reinterpret_cast<volatile uint32_t*>(
            static_cast<uintptr_t>(fb.address) + y * fb.pitch
        );
        row[x] = makeColor(fb, red, green, blue);
    }

    void fillScreen(const GRUB::Framebuffer& fb,
                    uint8_t red, uint8_t green, uint8_t blue) {
        for (uint32_t y = 0; y < fb.height; ++y) {
            for (uint32_t x = 0; x < fb.width; ++x) {
                putPixel(fb, x, y, red, green, blue);
            }
        }
    }

    extern "C" [[noreturn]] void kernel32(uint32_t mbMagic, uint32_t mbAddr) {
        if (GRUB::checkMultiboot(mbMagic)) {
            const auto* fb = GRUB::findFramebuffer(mbAddr);

            // The 32-bit kernel can directly access only framebuffers below 4 GiB.
            if (fb != nullptr && fb->framebufferType == 1 && fb->bpp == 32 &&
                (fb->address >> 32) == 0) {
                fillScreen(*fb, 20, 35, 70);

                for (uint32_t y = 100; y < 300; ++y) {
                    for (uint32_t x = 100; x < 500; ++x) {
                        putPixel(*fb, x, y, 59, 130, 246);
                    }
                }
            }
        }

        CPU::interrupts(false);
        CPU::halt();
    }
}
