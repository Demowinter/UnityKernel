#pragma once
#include <libelf/header.hpp>

namespace ELF {
#if UINTPTR_MAX == UINT32_MAX
    constexpr bool supportsELF32 = true;
    constexpr bool supportsELF64 = false;
#else
    constexpr bool supportsELF32 = true;
    constexpr bool supportsELF64 = true;
#endif

    ELFHeader parseHeader(void* addr);

    ELFSegmentEntry parseSegmentEntry(void* entryAddr, const ELFHeader& header);
    ELFSectionEntry parseSectionEntry(void* entryAddr, const ELFHeader& header);
}