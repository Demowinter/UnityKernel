#include <memory>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <libarch/api.hpp>
#include <librt/runtime.hpp>
#include <kernel/memory.hpp>
#include <kernel/console.hpp>
#include <libstd/function.hpp>
#include <libstd/string.hpp>
#include <libgrub/multiboot.hpp>

namespace Kernel {
    extern "C" [[noreturn]] void kernelMain(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        Console::info("Starting kernel32...");

        // Memory::initialize();
        CXXRuntime::initialize();

        if (!GRUB::checkMultiboot(mbMagic)) Console::info("Multiboot structure is corrupted");
        else Console::info("Multiboot structure is OK");

        Console::info("CPU manufacturer: ", false);
        Console::println(Arch::CPU::manufacturer(), 0x05);

        Console::ok("Started kernel32");

        Console::newline();

        Console::print("Welcome to ");
        Console::println("UnityKernel! v0.1.0-alpha", 5);

        STDLib::String str = "Hello, World!";
        STDLib::Function<void()> func = [str](){ Console::ok(str); };

        func();

        CXXRuntime::finalize();

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}
