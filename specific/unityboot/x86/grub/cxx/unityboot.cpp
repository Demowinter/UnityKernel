#include <libgrub/multiboot.hpp>
#include <libelf/file.hpp>
#include <libarch/api.hpp>
#include <libplatform/api.hpp>
#include <libstd/string.hpp>
#include <libstd/assert.hpp>
#include <libbase/math.hpp>
#include <unityboot/protocol.hpp>
#include <unityboot/console.hpp>
#include <unityboot/memory.hpp>

namespace UnityBoot {
    extern "C" [[noreturn]] void unityBootMain(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        Memory::initialize();
        Console::info("Hello from UnityBoot!");

        if (!GRUB::checkMultiboot(mbMagic, mbInfo)) {
            Console::fail("Multiboot structure is corrupted");

            Arch::Interrupt::disable();
            Arch::CPU::halt();
        }

        Console::ok("Multiboot structure is OK");

        GRUB::MultibootParser parser{mbInfo};

        UnityBootProtocol::BootInfo bootInfo{};
        bootInfo.arch = Arch::Type::x86;
        bootInfo.firmware = Platform::FirmwareType::BIOS;
        bootInfo.bootloader = Platform::BootloaderType::GRUB;
        
        for (auto& tag : parser) {
            switch (tag.type) {
                case GRUB::MultibootTagType::CommandLine: {
                    bootInfo.cmdline = reinterpret_cast<const GRUB::MBTags::CommandLineTag*>(&tag)->string;

                    break;
                }

                case GRUB::MultibootTagType::BootLoaderName: {
                    Console::info("Bootloader name: ", false);
                    Console::println(reinterpret_cast<const GRUB::MBTags::BootLoaderNameTag*>(&tag)->string);

                    break;
                }

                case GRUB::MultibootTagType::Module: {
                    auto moduleTag = reinterpret_cast<const GRUB::MBTags::ModuleTag*>(&tag);

                    bootInfo.kernelELFStart = moduleTag->start;
                    bootInfo.kernelELFEnd = moduleTag->end;

                    break;
                }

                case GRUB::MultibootTagType::Efi32:
                case GRUB::MultibootTagType::Efi64: bootInfo.firmware = Platform::FirmwareType::UEFI; break;

                case GRUB::MultibootTagType::AcpiOld:
                case GRUB::MultibootTagType::AcpiNew: bootInfo.hardwareDescriptionType = Platform::HDType::ACPI; break;
            }
        }

        ELF::ELFFile elf{reinterpret_cast<void*>(bootInfo.kernelELFStart)};

        if (elf.isValid()) {
            Console::ok("ELF Header is OK");

            Console::info("Loading ELF to memory...");

            elf.load();

            Console::ok("ELF loaded successfully");

            Memory::freeze();
            Memory::MemoryRegion region = Memory::getFreeMemoryRegion();

            bootInfo.heapStart = region.start;
            bootInfo.heapEnd = region.end;

            reinterpret_cast<void(*)(const UnityBootProtocol::BootInfo&)>(elf.entryAddress())(bootInfo); // Call kernelMain
        }

        else Console::fail("ELF header is corrupted");

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}