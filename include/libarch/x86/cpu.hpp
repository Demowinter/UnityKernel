#pragma once
#include <cstdint>
#include <libkstd/string.hpp>

namespace Arch::X86::CPU {
    struct Info {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    };

    Info cpuid(uint32_t leaf);

    KernelSTD::String manufacturer(const Info& info);

    [[noreturn]] void halt();
}