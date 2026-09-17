#pragma once
#include <cstddef>
#include <libelf/header.hpp>
#include <libelf/segment.hpp>
#include <libelf/section.hpp>
#include <elf.h>

namespace ELF {
    class ELFFile {
    public:
        ELFFile(void* addr);

        bool isValid();

        ELFClass elfclass();
        ELFEndian endian();
        ELFABI abi();

        uintptr_t entryAddress();

        uintptr_t segmentTableOffset();
        uintptr_t sectionTableOffset();

        size_t segmentEntrySize();
        size_t sectionEntrySize();

        size_t segmentEntryCount();
        size_t sectionEntryCount();

        ELFSegmentTable segments();
        ELFSectionTable sections();

        void dumpHeader();

    private:
        ELFHeader header;

        void* baseAddr;

        bool errorFlag;
    };
}