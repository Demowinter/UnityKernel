#include <optional>
#include <cstddef>
#include <cstdint>
#include <libbase/memory.hpp>
#include <liblltools/cursor.hpp>
#include <libelf/elf.hpp>

namespace ELF {
    std::optional<ELFHeader> parseHeader(void* addr) {
        LLTools::Cursor cursor{addr};

        ELFHeader header{};
        header.ident = cursor.read<ELFIdent>();

        if (memcmp(header.ident.magic, ELF::elfmagic, 4)) return std::nullopt;
        if (header.ident.endian != ELFEndian::LITTLE) return std::nullopt;

        header.type = cursor.read<ELFType>();
        header.machine = cursor.read<ELFMachine>();
        header.version = cursor.read<ELFVersion>();

        if (header.ident.elfclass == ELFClass::ELF64) {
            header.entry = cursor.read<uint64_t>();
            header.phoff = cursor.read<uint64_t>();
            header.shoff = cursor.read<uint64_t>();
        }

        else {
            header.entry = cursor.read<uint32_t>();
            header.phoff = cursor.read<uint32_t>();
            header.shoff = cursor.read<uint32_t>();
        }

        header.flags = cursor.read<uint32_t>();
        header.ehsize = cursor.read<uint16_t>();
        header.phentsize = cursor.read<uint16_t>();
        header.phnum = cursor.read<uint16_t>();
        header.shentsize = cursor.read<uint16_t>();
        header.shnum = cursor.read<uint16_t>();
        header.shstrtndx = cursor.read<uint16_t>();

        return header;
    }

    ELFProgramHeader parseProgramHeader(void* addr, ELFHeader& elfheader) {
        LLTools::Cursor cursor{addr};

        ELFProgramHeader header{};
        header.type = cursor.read<ELFProgramType>();

        if (elfheader.ident.elfclass == ELFClass::ELF64) {
            header.flags = cursor.read<uint32_t>();
            header.offset = cursor.read<uint64_t>();
            header.vaddr = cursor.read<uint64_t>();
            header.paddr = cursor.read<uint64_t>();
            header.fsize = cursor.read<uint64_t>();
            header.msize = cursor.read<uint64_t>();
            header.align = cursor.read<uint64_t>();
        }

        else {
            header.offset = cursor.read<uint32_t>();
            header.vaddr = cursor.read<uint32_t>();
            header.paddr = cursor.read<uint32_t>();
            header.fsize = cursor.read<uint32_t>();
            header.msize = cursor.read<uint32_t>();
            header.flags = cursor.read<uint32_t>();
            header.align = cursor.read<uint32_t>();
        }

        return header;
    }

    ELFSectionHeader parseSectionHeader(void* addr, ELFHeader& elfheader) {
        LLTools::Cursor cursor{addr};

        ELFSectionHeader header{};
        header.name = cursor.read<uint32_t>();
        header.type = cursor.read<ELFSectionType>();
        
        if (elfheader.ident.elfclass == ELFClass::ELF64) {
            header.flags = cursor.read<uint64_t>();
            header.addr = cursor.read<uint64_t>();
            header.offset = cursor.read<uint64_t>();
            header.size = cursor.read<uint64_t>();
        }

        else {
            header.flags = cursor.read<uint32_t>();
            header.addr = cursor.read<uint32_t>();
            header.offset = cursor.read<uint32_t>();
            header.size = cursor.read<uint32_t>();
        }

        header.link = cursor.read<uint32_t>();
        header.info = cursor.read<uint32_t>();

        if (elfheader.ident.elfclass == ELFClass::ELF64) {
            header.addralign = cursor.read<uint64_t>();
            header.entsize = cursor.read<uint64_t>();
        }

        else {
            header.addralign = cursor.read<uint32_t>();
            header.entsize = cursor.read<uint32_t>();
        }

        return header;
    }

    ELFParser::ELFParser(void* addr) : baseAddr{addr} {
        LLTools::Cursor cursor{addr};

        header.ident = cursor.read<ELFIdent>();

        errorFlag = memcmp(header.ident.magic, ELF::elfmagic, 4)
                    || header.ident.endian != ELFEndian::LITTLE
                    || header.ident.elfclass != ELFClass::ELF64
                    && header.ident.elfclass != ELFClass::ELF32;

        if (!errorFlag) {
            header.type = cursor.read<ELFType>();
            header.machine = cursor.read<ELFMachine>();
            header.version = cursor.read<ELFVersion>();

            if (header.ident.elfclass == ELFClass::ELF64) {
                header.entry = cursor.read<uint64_t>();
                header.phoff = cursor.read<uint64_t>();
                header.shoff = cursor.read<uint64_t>();
            }

            else if (header.ident.elfclass == ELFClass::ELF32) {
                header.entry = cursor.read<uint32_t>();
                header.phoff = cursor.read<uint32_t>();
                header.shoff = cursor.read<uint32_t>();
            }

            header.flags = cursor.read<uint32_t>();
            header.ehsize = cursor.read<uint16_t>();
            header.phentsize = cursor.read<uint16_t>();
            header.phnum = cursor.read<uint16_t>();
            header.shentsize = cursor.read<uint16_t>();
            header.shnum = cursor.read<uint16_t>();
            header.shstrtndx = cursor.read<uint16_t>();
        }
    }

    bool ELFParser::isValid() {
        return !errorFlag;
    }

    ELFClass ELFParser::elfclass() {
        return header.ident.elfclass;
    }

    ELFEndian ELFParser::endian() {
        return header.ident.endian;
    }

    ELFABI ELFParser::abi() {
        return header.ident.abi;
    }

    uintptr_t ELFParser::entryAddress() {
        return header.entry;
    }

    uintptr_t ELFParser::programHeaderOffset() {
        return header.phoff;
    }

    uintptr_t ELFParser::sectionHeaderOffset() {
        return header.shoff;
    }

    // bool ELFParser::parseHeader() {
    //     cursor.rseek(0);

    //     header.ident = cursor.read<ELFIdent>();

    //     if (memcmp(header.ident.magic, ELF::elfmagic, 4)) return true;
    //     if (header.ident.endian != ELFEndian::LITTLE) return true;

    //     header.type = cursor.read<ELFType>();
    //     header.machine = cursor.read<ELFMachine>();
    //     header.version = cursor.read<ELFVersion>();

    //     if (header.ident.elfclass == ELFClass::ELF64) {
    //         header.entry = cursor.read<uint64_t>();
    //         header.phoff = cursor.read<uint64_t>();
    //         header.shoff = cursor.read<uint64_t>();
    //     }

    //     else {
    //         header.entry = cursor.read<uint32_t>();
    //         header.phoff = cursor.read<uint32_t>();
    //         header.shoff = cursor.read<uint32_t>();
    //     }

    //     header.flags = cursor.read<uint32_t>();
    //     header.ehsize = cursor.read<uint16_t>();
    //     header.phentsize = cursor.read<uint16_t>();
    //     header.phnum = cursor.read<uint16_t>();
    //     header.shentsize = cursor.read<uint16_t>();
    //     header.shnum = cursor.read<uint16_t>();
    //     header.shstrtndx = cursor.read<uint16_t>();

    //     return false;
    // }

    // bool ELFParser::parseProgramHeader() {
    //     if (errorFlag) return true;
    //     if (header.ident.elfclass != ELFClass::ELF32 && header.ident.elfclass != ELFClass::ELF64) return true;

    //     cursor.rseek(header.phoff);

    //     pheader.type = cursor.read<ELFProgramType>();

    //     if (header.ident.elfclass == ELFClass::ELF64) {
    //         pheader.flags = cursor.read<uint32_t>();
    //         pheader.offset = cursor.read<uint64_t>();
    //         pheader.vaddr = cursor.read<uint64_t>();
    //         pheader.paddr = cursor.read<uint64_t>();
    //         pheader.fsize = cursor.read<uint64_t>();
    //         pheader.msize = cursor.read<uint64_t>();
    //         pheader.align = cursor.read<uint64_t>();
    //     }

    //     else if (header.ident.elfclass == ELFClass::ELF32) {
    //         pheader.offset = cursor.read<uint32_t>();
    //         pheader.vaddr = cursor.read<uint32_t>();
    //         pheader.paddr = cursor.read<uint32_t>();
    //         pheader.fsize = cursor.read<uint32_t>();
    //         pheader.msize = cursor.read<uint32_t>();
    //         pheader.flags = cursor.read<uint32_t>();
    //         pheader.align = cursor.read<uint32_t>();
    //     }

    //     return false;
    // }
    
    // bool ELFParser::parseSectionHeader() {
    //     if (errorFlag) return true;
    //     if (header.ident.elfclass != ELFClass::ELF32 && header.ident.elfclass != ELFClass::ELF64) return true;

    //     cursor.rseek(header.phoff);
    // }
}