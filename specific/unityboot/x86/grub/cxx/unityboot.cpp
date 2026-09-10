#include <libgrub/multiboot.hpp>
#include <libelf/elf.hpp>
#include <libarch/api.hpp>
#include <libstd/string.hpp>
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

        size_t moduleStart = 0;
        size_t moduleEnd = 0;
        
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

        auto header_ok = ELF::parseHeader(reinterpret_cast<void*>(moduleStart));

        if (header_ok) {
            Console::ok("ELF Header is OK");

            auto header = header_ok.value();

            ELF::dump(header);
        }

        else Console::fail("ELF header is corrupted");

        Arch::Interrupt::disable();
        Arch::CPU::halt();
    }
}