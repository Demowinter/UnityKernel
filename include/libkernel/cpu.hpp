#pragma once
#include <array>
#include <cstdint>

namespace Kernel::CPU {
    struct Info {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    };

    Info cpuid(uint32_t leaf);

    std::array<uint32_t, 3> manufacturer(const Info& info);

    void interrupts(bool on);
    [[noreturn]] void halt();
}