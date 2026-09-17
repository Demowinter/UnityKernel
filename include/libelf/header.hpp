#pragma once
#include <cstdint>
#include <elf.h>

namespace ELF {
    constexpr uint8_t elfmagic[] = {0x7F, 'E', 'L', 'F'};
    constexpr uint8_t elfversion = 1;
    constexpr uint16_t phentmax = 0xffff;
    constexpr uint16_t shentmax = 0xff00;

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

    enum class ELFSegmentType : uint32_t {
        NONE,
        LOAD,
        DYNAMIC,
        INTERP,
        NOTE,
        SHLIB,
        PHDR,
        TLS,
        NUM
    };

    enum class ELFSegmentFlag : uint32_t {
        EXEC = 0x01,
        WRITE = 0x02,
        READ = 0x04
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

    enum class ELFSectionFlag : uint64_t {
        WRITE   = 0x01,
        ALLOC   = 0x02,
        EXEC    = 0x04,
        MERGE   = 0x10,
        STRINGS = 0x20,
        INFO    = 0x40
    };

    struct ELFIdent {
        uint8_t magic[4];
        ELFClass elfclass;
        ELFEndian endian;
        uint8_t version;
        ELFABI abi;
        uint8_t abiversion;
        uint8_t valid;
        uint8_t padding[6];
    };

    struct ELFHeader {
        ELFIdent ident;     // Magic number and other info
        ELFType type;       // Object file type
        ELFMachine machine; // Architecture
        ELFVersion version; // Object file version
        uint64_t entry;     // Entry point function virtual address
        uint64_t phoff;     // Program header table offset
        uint64_t shoff;     // Section header table offset
        uint32_t flags;     // Processor-specific flags
        uint16_t ehsize;    // ELF header size in bytes
        uint16_t phentsize; // Program (segment) header table entry size
        uint16_t phnum;     // Program (segment) header table entry count
        uint16_t shentsize; // Section header table entry size
        uint16_t shnum;     // Section header table entry count
        uint16_t shstrtndx; // Section header string table index
    };

    struct ELFSegmentEntry {
        ELFSegmentType type;    // Segment type
        ELFSegmentFlag flags;   // Segment flags
        uint64_t offset;        // Segment file offset
        uint64_t vaddr;         // Segment virtual address
        uint64_t paddr;         // Segment physical address
        uint64_t fsize;         // Segment size in file
        uint64_t msize;         // Segment size in memory
        uint64_t align;         // Segment alignment
    };

    struct ELFSectionEntry {
        uint32_t name;          // Section name (string tbl index)
        ELFSectionType type;    // Section type
        ELFSectionFlag flags;   // Section flags
        uint64_t addr;          // Section virtual addr at execution
        uint64_t offset;        // Section file offset
        uint64_t size;          // Section size in bytes
        uint32_t link;          // Link to another section
        uint32_t info;          // Additional section information
        uint64_t addralign;     // Section alignment
        uint64_t entsize;       // Entry size if section holds table
    };
}