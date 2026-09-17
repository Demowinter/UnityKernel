#pragma once
#include <libelf/header.hpp>

namespace ELF {
    ELFHeader parseHeader(void* addr);

    ELFSegmentEntry parseSegmentEntry(void* entryAddr, const ELFHeader& header);
    ELFSectionEntry parseSectionEntry(void* entryAddr, const ELFHeader& header);
}