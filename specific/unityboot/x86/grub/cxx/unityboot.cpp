#include <libgrub/multiboot.hpp>
#include <libelf/file.hpp>
#include <libarch/api.hpp>
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

        STDLib::String cmdline;

        uintptr_t moduleStart = 0;
        uintptr_t moduleEnd = 0;
        
        for (auto& tag : parser) {
            switch (tag.type) {
                case GRUB::MultibootTagType::CommandLine: {
                    cmdline = reinterpret_cast<const GRUB::MBTags::CommandLineTag*>(&tag)->string;

                    break;
                }

                case GRUB::MultibootTagType::Module: {
                    auto moduleTag = reinterpret_cast<const GRUB::MBTags::ModuleTag*>(&tag);

                    moduleStart = moduleTag->start;
                    moduleEnd = moduleTag->end;

                    break;
                }
            }
        }

        Console::ok(cmdline);

        Console::info("Module start: ", false);
        Console::println(STDLib::toString(moduleStart));

        Console::info("Module end: ", false);
        Console::println(STDLib::toString(moduleEnd));

        ELF::ELFFile elf{reinterpret_cast<void*>(moduleStart)};

        if (elf.isValid()) {
            Console::ok("ELF Header is OK");

            elf.dumpHeader();

            Console::newline();

            Console::info("ELF segments dump:");

            for (auto segment : elf.segments()) {
                Console::info("Segment type: ", false);
                Console::println(STDLib::hex(static_cast<int>(segment.type)));

                Console::info("Segment file offset: ", false);
                Console::println(STDLib::hex(segment.offset));

                Console::info("Segment physical address: ", false);
                Console::println(STDLib::hex(segment.paddr));
                
                Console::info("Segment virtual address: ", false);
                Console::println(STDLib::hex(segment.vaddr));

                Console::info("Segment file size: ", false);
                Console::println(STDLib::toString(segment.fsize));

                Console::info("Segment memory size: ", false);
                Console::println(STDLib::toString(segment.msize));

                Console::newline();
            }

            Console::info("ELF sections dump:");

            for (auto section : elf.sections()) {
                Console::info("Section type: ", false);
                Console::println(STDLib::hex(static_cast<int>(section.type)));

                Console::info("Section file offset: ", false);
                Console::println(STDLib::hex(section.offset));

                Console::info("Section szie in bytes: ", false);
                Console::println(STDLib::toString(section.size));

                Console::newline();
            }

            Console::info("Loading ELF to memory...");

            elf.load();

            Console::ok("ELF loaded successfully");

            Console::info("Entry address: ", false);
            Console::println(STDLib::hex(elf.entryAddress()));

            Memory::freeze();

            Memory::MemoryRegion region = Memory::getFreeMemoryRegion();

            UnityBootProtocol::Info info;
            info.memoryRegion.start = region.start;
            info.memoryRegion.end = region.end;
            info.cmdline = cmdline.data();

            reinterpret_cast<void(*)(const UnityBootProtocol::Info&)>(elf.entryAddress())(info); // Call kernelMain
        }

        else Console::fail("ELF header is corrupted");

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}