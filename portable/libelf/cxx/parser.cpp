#include <optional>
#include <cstddef>
#include <cstdint>
#include <libbase/memory.hpp>
#include <liblltools/cursor.hpp>
#include <libelf/elf.hpp>

namespace ELF {

    namespace {
        template <typename T>
        T read(LLTools::Cursor& cursor) {
            T value{};
            cursor.read(&value, sizeof(T));
            return value;
        }
    }

    std::optional<ELFHeader> parseHeader(void* addr) {
        LLTools::Cursor cursor{addr};

        ELFHeader header{};
        header.ident = read<ELFIdent>(cursor);

        if (memcmp(header.ident.magic, ELF::elfmagic, 4)) return std::nullopt;
        if (header.ident.endian != ELFEndian::LITTLE) return std::nullopt;

        header.type = read<ELFType>(cursor);
        header.machine = read<ELFMachine>(cursor);
        header.version = read<ELFVersion>(cursor);

        if (header.ident.elfclass == ELFClass::ELF64) {
            header.entry = read<uint64_t>(cursor);
            header.phoff = read<uint64_t>(cursor);
            header.shoff = read<uint64_t>(cursor);
        }

        else {
            header.entry = read<uint32_t>(cursor);
            header.phoff = read<uint32_t>(cursor);
            header.shoff = read<uint32_t>(cursor);
        }

        header.flags = read<uint32_t>(cursor);
        header.ehsize = read<uint16_t>(cursor);
        header.phentsize = read<uint16_t>(cursor);
        header.phnum = read<uint16_t>(cursor);
        header.shentsize = read<uint16_t>(cursor);
        header.shnum = read<uint16_t>(cursor);
        header.shstrtndx = read<uint16_t>(cursor);

        return header;
    }

    ELFProgramHeader parseProgramHeader(void* addr, ELFHeader& elfheader) {
        LLTools::Cursor cursor{addr};

        ELFProgramHeader header{};
        header.type = read<ELFProgramType>(cursor);

        if (elfheader.ident.elfclass == ELFClass::ELF64) {
            header.flags = read<uint32_t>(cursor);
            header.offset = read<uint64_t>(cursor);
            header.vaddr = read<uint64_t>(cursor);
            header.paddr = read<uint64_t>(cursor);
            header.fsize = read<uint64_t>(cursor);
            header.msize = read<uint64_t>(cursor);
            header.align = read<uint64_t>(cursor);
        }

        else {
            header.offset = read<uint32_t>(cursor);
            header.vaddr = read<uint32_t>(cursor);
            header.paddr = read<uint32_t>(cursor);
            header.fsize = read<uint32_t>(cursor);
            header.msize = read<uint32_t>(cursor);
            header.flags = read<uint32_t>(cursor);
            header.align = read<uint32_t>(cursor);
        }

        return header;
    }

    ELFSectionHeader parseSectionHeader(void* addr, ELFHeader& elfheader) {
        LLTools::Cursor cursor{addr};

        ELFSectionHeader header{};
        header.name = read<uint32_t>(cursor);
        header.type = read<ELFSectionType>(cursor);

        if (elfheader.ident.elfclass == ELFClass::ELF64) {
            header.flags = read<uint64_t>(cursor);
            header.addr = read<uint64_t>(cursor);
            header.offset = read<uint64_t>(cursor);
            header.size = read<uint64_t>(cursor);
        }

        else {
            header.flags = read<uint32_t>(cursor);
            header.addr = read<uint32_t>(cursor);
            header.offset = read<uint32_t>(cursor);
            header.size = read<uint32_t>(cursor);
        }

        header.link = read<uint32_t>(cursor);
        header.info = read<uint32_t>(cursor);

        if (elfheader.ident.elfclass == ELFClass::ELF64) {
            header.addralign = read<uint64_t>(cursor);
            header.entsize = read<uint64_t>(cursor);
        }

        else {
            header.addralign = read<uint32_t>(cursor);
            header.entsize = read<uint32_t>(cursor);
        }

        return header;
    }

    ELFHeaderParser::ELFHeaderParser(void* addr) : baseAddr{addr} {
        LLTools::Cursor cursor{addr};

        header.ident = read<ELFIdent>(cursor);

        errorFlag = memcmp(header.ident.magic, ELF::elfmagic, 4)
                    || header.ident.endian != ELFEndian::LITTLE
                    || header.ident.elfclass != ELFClass::ELF64
                    && header.ident.elfclass != ELFClass::ELF32;

        if (!errorFlag) {
            header.type = read<ELFType>(cursor);
            header.machine = read<ELFMachine>(cursor);
            header.version = read<ELFVersion>(cursor);

            if (header.ident.elfclass == ELFClass::ELF64) {
                header.entry = read<uint64_t>(cursor);
                header.phoff = read<uint64_t>(cursor);
                header.shoff = read<uint64_t>(cursor);
            }

            else if (header.ident.elfclass == ELFClass::ELF32) {
                header.entry = read<uint32_t>(cursor);
                header.phoff = read<uint32_t>(cursor);
                header.shoff = read<uint32_t>(cursor);
            }

            header.flags = read<uint32_t>(cursor);
            header.ehsize = read<uint16_t>(cursor);
            header.phentsize = read<uint16_t>(cursor);
            header.phnum = read<uint16_t>(cursor);
            header.shentsize = read<uint16_t>(cursor);
            header.shnum = read<uint16_t>(cursor);
            header.shstrtndx = read<uint16_t>(cursor);
        }
    }

    bool ELFHeaderParser::isValid() {
        return !errorFlag;
    }

    ELFClass ELFHeaderParser::elfclass() {
        return header.ident.elfclass;
    }

    ELFEndian ELFHeaderParser::endian() {
        return header.ident.endian;
    }

    ELFABI ELFHeaderParser::abi() {
        return header.ident.abi;
    }

    uintptr_t ELFHeaderParser::entryAddress() {
        return header.entry;
    }

    uintptr_t ELFHeaderParser::programHeaderOffset() {
        return header.phoff;
    }

    uintptr_t ELFHeaderParser::sectionHeaderOffset() {
        return header.shoff;
    }
}