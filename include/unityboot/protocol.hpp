#pragma once
#include <cstddef>

namespace UnityBootProtocol{
    struct MemoryRegion {
        uintptr_t start;
        uintptr_t end;
    };

    struct Info {
        MemoryRegion memoryRegion;
        char* cmdline;
    };
}