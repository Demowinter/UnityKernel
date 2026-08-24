#include <libgrub/multiboot.hpp>
#include <libkernel/console.hpp>
#include <libarch/api.hpp>
#include <libuniboot/protocol.hpp>

namespace UniBoot {
    extern "C" [[noreturn]] void unibootMain() {
        Kernel::Console::info("Hello from UniBoot!");

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}