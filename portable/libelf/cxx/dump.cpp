#include <libenv/tty.hpp>
#include <libstd/string.hpp>
#include <libelf/header.hpp>
#include <libelf/utils.hpp>

namespace ELF::Utils {
    void dump(const ELFHeader& header) {
        ENV::TTY::info("ELF Header");

        ENV::TTY::info("  Magic:       ", false);
        ENV::TTY::println(
            std::string_view(
                reinterpret_cast<const char*>(header.ident.magic),
                sizeof(header.ident.magic)
            )
        );

        ENV::TTY::info("  Class:       ", false);
        switch (header.ident.elfclass) {
            case ELFClass::ELF32:
                ENV::TTY::println("ELF32");
                break;

            case ELFClass::ELF64:
                ENV::TTY::println("ELF64");
                break;

            default:
                ENV::TTY::println("NONE");
                break;
        }

        ENV::TTY::info("  Endian:      ", false);
        switch (header.ident.endian) {
            case ELFEndian::LITTLE:
                ENV::TTY::println("Little Endian");
                break;

            case ELFEndian::BIG:
                ENV::TTY::println("Big Endian");
                break;

            default:
                ENV::TTY::println("NONE");
                break;
        }

        ENV::TTY::info("  Version:     ", false);
        switch (header.ident.version) {
            case 1:
                ENV::TTY::println("Current");
                break;

            default:
                ENV::TTY::println("Invalid");
                break;
        }

        ENV::TTY::info("  ABI:         ", false);
        switch (header.ident.abi) {
            case ELFABI::SYSV:
                ENV::TTY::println("System V");
                break;

            case ELFABI::HPUX:
                ENV::TTY::println("HP-UX");
                break;

            case ELFABI::NETBSD:
                ENV::TTY::println("NetBSD");
                break;

            case ELFABI::GNU:
                ENV::TTY::println("GNU/Linux");
                break;

            case ELFABI::SOLARIS:
                ENV::TTY::println("Solaris");
                break;

            case ELFABI::AIX:
                ENV::TTY::println("AIX");
                break;

            case ELFABI::IRIX:
                ENV::TTY::println("IRIX");
                break;

            case ELFABI::FREEBSD:
                ENV::TTY::println("FreeBSD");
                break;

            case ELFABI::OPENBSD:
                ENV::TTY::println("OpenBSD");
                break;

            case ELFABI::ARM_EABI:
                ENV::TTY::println("ARM EABI");
                break;

            case ELFABI::ARM:
                ENV::TTY::println("ARM");
                break;

            case ELFABI::STANDALONE:
                ENV::TTY::println("Standalone");
                break;

            default:
                ENV::TTY::println("Unknown");
                break;
        }

        ENV::TTY::info("  Type:        ", false);
        switch (header.type) {
            case ELFType::NONE:  ENV::TTY::println("NONE"); break;
            case ELFType::RELOC: ENV::TTY::println("REL");  break;
            case ELFType::EXEC:  ENV::TTY::println("EXEC"); break;
            case ELFType::DYN:   ENV::TTY::println("DYN");  break;
            case ELFType::CORE:  ENV::TTY::println("CORE"); break;
            default:             ENV::TTY::println("UNKNOWN"); break;
        }

        ENV::TTY::info("  Machine:     ", false);
        switch (header.machine) {
            case ELFMachine::NONE:   ENV::TTY::println("None");   break;
            case ELFMachine::I386:   ENV::TTY::println("i386");   break;
            case ELFMachine::X86_64: ENV::TTY::println("x86-64"); break;
            case ELFMachine::ARM:    ENV::TTY::println("ARM");    break;
            case ELFMachine::AARCH64:ENV::TTY::println("AArch64");break;
            case ELFMachine::RISCV:  ENV::TTY::println("RISC-V"); break;
            default:                 ENV::TTY::println("Unknown"); break;
        }

        ENV::TTY::info("  Entry:        ", false);
        ENV::TTY::println(STDLib::hex(header.entry));

        ENV::TTY::info("  PH offset:    ", false);
        ENV::TTY::println(STDLib::hex(header.phoff));

        ENV::TTY::info("  SH offset:    ", false);
        ENV::TTY::println(STDLib::hex(header.shoff));

        ENV::TTY::info("  Flags:        ", false);
        ENV::TTY::println(STDLib::hex(header.flags));

        ENV::TTY::info("  PH entry size: ", false);
        ENV::TTY::println(STDLib::hex(header.phentsize));

        ENV::TTY::info("  PH num:        ", false);
        ENV::TTY::println(STDLib::toString(header.phnum));

        ENV::TTY::info("  SH entry size: ", false);
        ENV::TTY::println(STDLib::hex(header.shentsize));

        ENV::TTY::info("  SH num:        ", false);
        ENV::TTY::println(STDLib::toString(header.shnum));
    }
}