#include <cstddef>
#include <libelf/header.hpp>
#include <libelf/parser.hpp>
#include <libelf/utils.hpp>
#include <libelf/file.hpp>

namespace ELF {
    ELFFile::ELFFile(void* addr) : baseAddr{addr} {
        auto result = parseHeader(addr);

        errorFlag = !result.has_value();
        header = result.value_or(ELFHeader{});
    }

    bool ELFFile::isValid() {
        return !errorFlag;
    }

    ELFClass ELFFile::elfclass() {
        return header.ident.elfclass;
    }

    ELFEndian ELFFile::endian() {
        return header.ident.endian;
    }

    ELFABI ELFFile::abi() {
        return header.ident.abi;
    }

    uintptr_t ELFFile::entryAddress() {
        return header.entry;
    }

    uintptr_t ELFFile::segmentTableOffset() {
        return header.phoff;
    }

    uintptr_t ELFFile::sectionTableOffset() {
        return header.shoff;
    }

    size_t ELFFile::segmentEntrySize() {
        return header.phentsize;
    }

    size_t ELFFile::sectionEntrySize() {
        return header.shentsize;
    }

    size_t ELFFile::segmentEntryCount() {
        return header.phnum;
    }

    size_t ELFFile::sectionEntryCount() {
        return header.shnum;
    }

    ELFSegmentTable ELFFile::segments() {
        return ELFSegmentTable{header, baseAddr};
    }

    ELFSectionTable ELFFile::sections() {
        return ELFSectionTable{header, baseAddr};
    }

    void ELFFile::dumpHeader() {
        Utils::dump(header);
    }
}