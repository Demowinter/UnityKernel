#include <string_view>
#include <libarch/api.hpp>
#include <libarch/x86/pmio.hpp>
#include <kernel/console.hpp>
#include <kernel/system.hpp>

namespace Kernel::System {
    [[noreturn]] void panic(std::string_view who, std::string_view what) {
        Console::newline();
        Console::println("==== KERNEL PANIC ====", 0x0C);
        Console::newline();

        Console::print("Caller: ", 0x0C);
        Console::println(who);

        Console::print("Reason: ", 0x0C);
        Console::println(what);
        
        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }

    [[noreturn]] void reboot() {
        //Try to reboot using 8042 keyboard controller
        Arch::X86::PMIO::write<uint8_t>(0x64, 0xFE);
        
        //Fallback: just halt if reboot fails
        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}