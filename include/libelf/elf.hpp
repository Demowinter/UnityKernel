#pragma once
#include <optional>
#include <cstddef>
#include <cstdint>
#include <libstd/string.hpp>
#include <libenv/tty.hpp>
#include <elf.h>

namespace ELF {
    enum class ELFClass : uint8_t {
        NONE,
        ELF32,
        ELF64
    };

    enum class ELFEndian : uint8_t {
        NONE,
        LITTLE,
        BIG
    };

    enum class ELFABI : uint8_t {
        NONE        = 0U,
        SYSV        = NONE,
        HPUX        = 1U,
        NETBSD      = 2U,
        GNU         = 3U,
        LINUX       = GNU,
        SOLARIS     = 6U,
        AIX         = 7U,
        IRIX        = 8U,
        FREEBSD     = 9U,
        TRU64       = 10U,
        MODESTO     = 11U,
        OPENBSD     = 12U,
        ARM_EABI    = 64U,
        ARM         = 97U,
        STANDALONE  = 255U
    };

    enum class ELFType : uint16_t {
        NONE,
        RELOC,
        EXEC,
        DYN,
        CORE
    };

    enum class ELFMachine : uint16_t {
        NONE        = 0U,
        SPARC       = 2U,
        I386        = 3U,
        M68K        = 4U,
        MIPS        = 8U,
        PPC         = 20U,
        PPC64       = 21U,
        S390        = 22U,
        ARM         = 40U,
        SPARCV9     = 43U,
        IA64        = 50U,
        X86_64      = 62U,
        AVR         = 83U,
        MSP430      = 105U,
        AARCH64     = 183U,
        CUDA        = 190U,
        AMDGPU      = 224U,
        RISCV       = 243U
    };

    enum class ELFVersion : uint32_t {
        NONE,
        CURRENT,
        NUM
    };

    enum class ELFProgramType : uint32_t {
        NONE,

    };

    enum class ELFSectionType : uint32_t {
        NONE            = 0,
        PROGBITS        = 1,
        SYMTAB          = 2,
        STRTAB          = 3,
        RELA            = 4,
        HASH            = 5,
        DYNAMIC         = 6,
        NOTE            = 7,
        NOBITS          = 8,
        REL             = 9,
        SHLIB           = 10,
        DYNSYM          = 11,
        INIT_ARRAY      = 14,
        FINI_ARRAY      = 15,
        PREINIT_ARRAY   = 16,
        GROUP           = 17,
        SYMTAB_SHNDX    = 18,
        RELR            = 19,
        NUM             = 20,
        GNU_ATTRIBUTES  = 0x6ffffff5,
        GNU_HASH        = 0x6ffffff6,
        GNU_LIBLIST     = 0x6ffffff7,
        CHECKSUM        = 0x6ffffff8,
        GNU_VERDEF      = 0x6ffffffd,
        GNU_VERNEED     = 0x6ffffffe,
        GNU_VERSYM      = 0x6fffffff
    };

    constexpr uint8_t elfmagic[] = {0x7F, 'E', 'L', 'F'};
    constexpr uint8_t elfversion = 1;
    constexpr uint16_t phentmax = 0xffff;
    constexpr uint16_t shentmax = 0xff00;

    struct ELFIdent {
        uint8_t magic[4];
        ELFClass elfclass;
        ELFEndian endian;
        uint8_t version;
        ELFABI abi;
        uint8_t abiversion;
        uint8_t padding[7];
    };

    struct ELFHeader {
        ELFIdent ident;
        ELFType type;
        ELFMachine machine;
        ELFVersion version;
        uint64_t entry;     // Entry point function virtual address
        uint64_t phoff;     // Program header table offset
        uint64_t shoff;     // Section header table offset
        uint32_t flags;     // Processor-specific flags
        uint16_t ehsize;    // ELF header size in bytes
        uint16_t phentsize; // Program header table entry size
        uint16_t phnum;     // Program header table entry count
        uint16_t shentsize; // Section header table entry size
        uint16_t shnum;     // Section header table entry count
        uint16_t shstrtndx; // Section header string table index
    };

    struct ELFProgramHeader {
        ELFProgramType type;
        uint32_t flags;
        uint64_t offset;
        uint64_t vaddr;
        uint64_t paddr;
        uint64_t fsize;
        uint64_t msize;
        uint64_t align;
    };

    struct ELFSectionHeader {
        uint32_t name;
        ELFSectionType type;
        uint64_t flags;
        uint64_t addr;
        uint64_t offset;
        uint64_t size;
        uint32_t link;
        uint32_t info;
        uint64_t addralign;
        uint64_t entsize;
    };

    // struct ELF

    static_assert(sizeof(ELFHeader) == 64);

    std::optional<ELFHeader> parseHeader(void* addr);
    // ELFProgramHeader parseProgramHeader(void* addr, ELFHeader& elfheader);
    // ELFSectionHeader parseSectionHeader(void* addr, ELFHeader& elfheader);

    class ELFParser {
    public:
        ELFParser(void* addr);

        bool isValid();

        ELFClass elfclass();
        ELFEndian endian();
        ELFABI abi();

        uintptr_t entryAddress();

        uintptr_t programHeaderOffset();
        uintptr_t sectionHeaderOffset();

    private:
        void* baseAddr;
        
        ELFHeader header;

        bool errorFlag;
    };

    // class ELFProgramIterator {
    // public:

    // private:
    // };

    // class ELFSectionIterator {
    // public:

    // private:
    // };

    class ELFProgramParser {
    public:

    private:
        size_t phoff;     // Program header table offset

        uint16_t phentsize; // Program header table entry size
        uint16_t phnum;     // Program header table entry count
    };

    class ELFSectionParser {
    public:

    private:
        size_t shoff;     // Section header table offset

        uint16_t shentsize; // Section header table entry size
        uint16_t shnum;     // Section header table entry count
    };

    class ELFHeaderParser {
    public:

    private:
        ELFIdent ident;
        ELFHeader header;
    };

    class ELFLoader {

    };

    inline STDLib::String hex(uint64_t value) {
        return "0x" + STDLib::toString(value, 16);
    }

    inline void dump(const ELFHeader& header) {
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

        // ENV::TTY::info("  ABI version: ", false);
        // println числа тут

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
        ENV::TTY::println(hex(header.entry));

        ENV::TTY::info("  PH offset:    ", false);
        ENV::TTY::println(hex(header.phoff));

        ENV::TTY::info("  SH offset:    ", false);
        ENV::TTY::println(hex(header.shoff));

        ENV::TTY::info("  Flags:        ", false);
        ENV::TTY::println(hex(header.flags));
    }
}