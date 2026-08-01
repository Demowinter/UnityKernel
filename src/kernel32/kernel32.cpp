#include <cstdint>
#include <cstddef>
#include <libkernel/cpu.hpp>
#include <libkernel/console.hpp>
#include <kernel32/grub.hpp>

namespace Kernel {    
    extern "C" [[noreturn]] void kernel32(uint32_t mbMagic, uint32_t mbAddr) {
        if (GRUB::checkMultiboot(mbMagic)) {
            const auto* fb = GRUB::findFramebuffer(mbAddr);

            if (fb != nullptr && fb->framebufferType == 1 && fb->bpp == 32 &&
                (fb->address >> 32) == 0) {
                Console::initialize({
                    static_cast<uintptr_t>(fb->address), fb->pitch, fb->width, fb->height,
                    fb->bpp, fb->framebufferType, fb->redPosition, fb->redMaskSize,
                    fb->greenPosition, fb->greenMaskSize, fb->bluePosition, fb->blueMaskSize,
                });
                Console::clear();
                Console::println("UnityKernel framebuffer console", 0x0B);
                Console::println("Multiboot2 framebuffer: OK", 0x0A);
            }
        }

        CPU::interrupts(false);
        CPU::halt();
    }
}
