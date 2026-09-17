#include <cstdint>
#include <libbase/memory.hpp>
#include <liblltools/cursor.hpp>
#include <libelf/header.hpp>
#include <libelf/parser.hpp>

namespace ELF {
    ELFHeader parseHeader(void* addr) {
        LLTools::Cursor cursor{addr};

        ELFHeader header{};
        header.ident = cursor.read<ELFIdent>();

        bool errorFlag = memcmp(header.ident.magic, ELF::elfmagic, 4)
                            || header.ident.endian != ELFEndian::LITTLE
                            || header.ident.elfclass != ELFClass::ELF64
                            && header.ident.elfclass != ELFClass::ELF32;

        header.ident.valid = !errorFlag;

        if (errorFlag) return header;

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

        return header;
    }

    ELFSegmentEntry parseSegmentEntry(void* entryAddr, const ELFHeader& header) {
        LLTools::Cursor cursor{entryAddr};

        ELFSegmentEntry entry{};
        entry.type = cursor.read<ELFSegmentType>();

        if (header.ident.elfclass == ELFClass::ELF64) {
            entry.flags = cursor.read<ELFSegmentFlag>();
            entry.offset = cursor.read<uint64_t>();
            entry.vaddr = cursor.read<uint64_t>();
            entry.paddr = cursor.read<uint64_t>();
            entry.fsize = cursor.read<uint64_t>();
            entry.msize = cursor.read<uint64_t>();
            entry.align = cursor.read<uint64_t>();
        }

        else {
            entry.offset = cursor.read<uint32_t>();
            entry.vaddr = cursor.read<uint32_t>();
            entry.paddr = cursor.read<uint32_t>();
            entry.fsize = cursor.read<uint32_t>();
            entry.msize = cursor.read<uint32_t>();
            entry.flags = cursor.read<ELFSegmentFlag>();
            entry.align = cursor.read<uint32_t>();
        }

        return entry;
    }

    ELFSectionEntry parseSectionEntry(void* entryAddr, const ELFHeader& header) {
        LLTools::Cursor cursor{entryAddr};

        ELFSectionEntry entry{};
        entry.name = cursor.read<uint32_t>();
        entry.type = cursor.read<ELFSectionType>();
        
        if (header.ident.elfclass == ELFClass::ELF64) {
            entry.flags = cursor.read<ELFSectionFlag>();
            entry.addr = cursor.read<uint64_t>();
            entry.offset = cursor.read<uint64_t>();
            entry.size = cursor.read<uint64_t>();
        }

        else {
            entry.flags = cursor.read<ELFSectionFlag>();
            entry.addr = cursor.read<uint32_t>();
            entry.offset = cursor.read<uint32_t>();
            entry.size = cursor.read<uint32_t>();
        }

        entry.link = cursor.read<uint32_t>();
        entry.info = cursor.read<uint32_t>();

        if (header.ident.elfclass == ELFClass::ELF64) {
            entry.addralign = cursor.read<uint64_t>();
            entry.entsize = cursor.read<uint64_t>();
        }

        else {
            entry.addralign = cursor.read<uint32_t>();
            entry.entsize = cursor.read<uint32_t>();
        }

        return entry;
    }
}