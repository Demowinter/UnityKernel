#pragma once
#include <cstdint>

namespace Kernel::CPU {
    struct Info {
        uint32_t eax;
        uint32_t ebx;
        uint32_t edx;
        uint32_t ecx;
    };

    Info cpuid(uint32_t leaf);

    void interrupts(bool on);

    void halt();
}