#pragma once
#include <optional>
#include <libelf/header.hpp>

namespace ELF {
    std::optional<ELFHeader> parseHeader(void* addr);

    ELFSegmentEntry parseSegmentEntry(void* entryAddr, const ELFHeader& header);
    ELFSectionEntry parseSectionEntry(void* entryAddr, const ELFHeader& header);
}