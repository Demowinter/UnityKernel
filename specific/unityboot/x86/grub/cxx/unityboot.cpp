#include <libgrub/multiboot.hpp>
#include <libarch/api.hpp>
#include <unityboot/protocol.hpp>
#include <unityboot/console.hpp>

namespace UnityBoot {
    extern "C" [[noreturn]] void unityBootMain(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        Console::info("Hello from UnityBoot!");

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