#pragma once
#include <cstddef>
#include <libarch/arch.hpp>
#include <libplatform/platform.hpp>

namespace UnityBootProtocol {
    struct BootInfo {
        Arch::Type arch;
        Platform::Info platform;

        uintptr_t kernelELFStart;
        uintptr_t kernelELFEnd;

        uintptr_t heapStart;
        uintptr_t heapEnd;
        
        const char* cmdline;
        const void* acpiRsdp;
        size_t acpiRsdpSize;
    };
}