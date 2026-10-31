#include <libgrub/multiboot.hpp>
#include <libelf/file.hpp>
#include <libarch/arch.hpp>
#include <libarch/cpu.hpp>
#include <libarch/interrupt.hpp>
#include <libarch/x86/gdt.hpp>
#include <libarch/x86/cpu.hpp>
#include <libplatform/platform.hpp>
#include <unityboot/protocol.hpp>
#include <unityboot/console.hpp>
#include <unityboot/memory.hpp>

namespace UnityBoot {
    extern "C" [[noreturn]] void bootMain(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
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
        bootInfo.platform.firmware = Platform::FirmwareType::BIOS;
        bootInfo.platform.bootloader = Platform::BootloaderType::GRUB;
        bool hasNewAcpiRSDP = false;
        
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
                case GRUB::MultibootTagType::Efi64: bootInfo.platform.firmware = Platform::FirmwareType::UEFI; break;

                case GRUB::MultibootTagType::AcpiOld:
                case GRUB::MultibootTagType::AcpiNew: {
                    const bool isNewRSDP = tag.type == GRUB::MultibootTagType::AcpiNew;

                    if (!hasNewAcpiRSDP || isNewRSDP) {
                        bootInfo.platform.hdt = Platform::HDType::ACPI;
                        bootInfo.acpiRsdp = reinterpret_cast<const uint8_t*>(&tag) + sizeof(GRUB::MultibootTag);
                        bootInfo.acpiRsdpSize = tag.size - sizeof(GRUB::MultibootTag);
                        hasNewAcpiRSDP = isNewRSDP;
                    }

                    break;
                }

                default: break;
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