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

    ELFProgramHeader parseProgramHeader(void* addr, ELFClass elfclass) {
        LLTools::Cursor cursor{addr};

        ELFProgramHeader header{};
        header.type = cursor.read<ELFProgramType>();

        if (elfclass == ELFClass::ELF64) {
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

    ELFSectionHeader parseSectionHeader(void* addr, ELFClass elfclass) {
        LLTools::Cursor cursor{addr};

        ELFSectionHeader header{};
        header.name = cursor.read<uint32_t>();
        header.type = cursor.read<ELFSectionType>();
        
        if (elfclass == ELFClass::ELF64) {
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

        if (elfclass == ELFClass::ELF64) {
            header.addralign = cursor.read<uint64_t>();
            header.entsize = cursor.read<uint64_t>();
        }

        else {
            header.addralign = cursor.read<uint32_t>();
            header.entsize = cursor.read<uint32_t>();
        }

        return header;
    }
}