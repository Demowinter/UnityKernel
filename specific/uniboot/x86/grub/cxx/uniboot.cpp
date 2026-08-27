#include <libgrub/multiboot.hpp>
#include <libarch/api.hpp>
#include <uniboot/protocol.hpp>
#include <uniboot/console.hpp>

namespace UniBoot {
    extern "C" [[noreturn]] void unibootMain(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        Console::info("Hello from UniBoot!");

        if (!GRUB::checkMultiboot(mbMagic)) {
            Console::fail("Multiboot structure is corrupted");

            Arch::Interrupt::disable();
            Arch::CPU::halt();
        }

        Console::ok("Multiboot structure is OK");

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}