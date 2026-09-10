#pragma once
#include <cstdint>
#include <elf.h>

enum class ELFArch : uint8_t {
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
    CURRENT
};

constexpr uint8_t elfmagic[] = {0x7F, 'E', 'L', 'F'};
constexpr uint8_t elfversion = 1;
constexpr uint16_t phentmax = 0xffff;
constexpr uint16_t shentmax = 0xff00;

struct ELFIdent {
    uint8_t magic[4];
    ELFArch arch;
    ELFEndian endian;
    ELFVersion version;
    ELFABI abi;
    uint8_t abiversion;
    uint8_t padding;
};

struct ELF32Header {
    ELFType type;
    ELFMachine machine;
    ELFVersion version;
    uint32_t entry;     // Entry point function virtual address
    uint32_t phoff;     // Program header table offset
    uint32_t shoff;     // Section header table offset
    uint32_t flags;     // Processor-specific flags
    uint16_t ehsize;    // ELF header size in bytes
    uint16_t phentsize; // Program header table entry size
    uint16_t phnum;     // Program header table entry count
    uint16_t shentsize; // Section header table entry size
    uint16_t shnum;     // Section header table entry count
    uint16_t shstrtndx; // Section header string table index
};

struct ELF64Header {
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

