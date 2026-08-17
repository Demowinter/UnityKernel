#include <string_view>
#include <libarch/api.hpp>
#include <libkernel/console.hpp>
#include <libkernel/system.hpp>

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
}