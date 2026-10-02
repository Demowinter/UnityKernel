#include <libarch/api.hpp>
#include <librt/runtime.hpp>
#include <libacpi/acpi.hpp>
#include <unityboot/protocol.hpp>
#include <kernel/heap.hpp>
#include <kernel/subsystems/driver.hpp>
#include <kernel/console.hpp>
#include <kernel/shell.hpp>
#include <kernel/fat32.hpp>

namespace Kernel {
    extern "C" [[noreturn]] void kernelMain(const UnityBootProtocol::BootInfo& info) {
        Console::clear();

        Console::info("Starting kernel32...");

        Heap::initialize(info.heapStart, info.heapEnd);
        CXXRuntime::initialize();

        Arch::initialize();
        DriverSubsystem::initialize();

        FAT32::initialize();

        if (Driver::ACPI::initialize(info.acpiRsdp, info.acpiRsdpSize)) {
            Console::ok("ACPI tables initialized");
        } else {
            Console::warn("ACPI tables unavailable or invalid");
        }

        Console::info("Memory region:");
        Console::info("        start: ", false);
        Console::println(STDLib::hex(info.heapStart));
        Console::info("          end: ", false);
        Console::println(STDLib::hex(info.heapEnd));

        Console::newline();

        Console::info("CPU manufacturer: ", false);
        Console::println(Arch::CPU::manufacturer(), 0x05);

        Console::newline();

        Console::print("Welcome to ");
        Console::println("UnityKernel! v0.1.0-alpha", 5);
        Console::ok("Mounted FAT32 ram disk");

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
