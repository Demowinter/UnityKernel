#include <memory>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <libarch/api.hpp>
#include <librt/runtime.hpp>
#include <kernel/memory.hpp>
#include <kernel/console.hpp>
#include <kernel/shell.hpp>
#include <libstd/function.hpp>
#include <libstd/string.hpp>
#include <unityboot/protocol.hpp>

namespace Kernel {
    extern "C" [[noreturn]] void kernelMain(const UnityBootProtocol::Info& info) {
        Console::clear();

        Console::info("Starting kernel32...");

        Memory::initialize(info.memoryRegion.start, info.memoryRegion.end);
        CXXRuntime::initialize();

        Arch::initialize();

        Console::info("Memory region:");
        Console::info("        start: ", false);
        Console::println(STDLib::hex(info.memoryRegion.start));
        Console::info("          end: ", false);
        Console::println(STDLib::hex(info.memoryRegion.end));

        Console::newline();

        Console::info("CPU manufacturer: ", false);
        Console::println(Arch::CPU::manufacturer(), 0x05);

        Console::newline();

        Console::print("Welcome to ");
        Console::println("UnityKernel! v0.1.0-alpha", 5);

        Console::newline();
        Console::println("Starting shell...");
        Console::newline();

        // Start the interactive shell
        Shell::run(info.cmdline);

        CXXRuntime::finalize();

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}
