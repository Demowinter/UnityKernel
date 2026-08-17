#pragma once
#include <cstdint>

namespace Arch::X86::MMIO {
    struct Region {
        uintptr_t start;
        uintptr_t end;
    };
}