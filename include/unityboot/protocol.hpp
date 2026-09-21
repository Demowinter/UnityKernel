#pragma once
#include <cstddef>
#include <libarch/api.hpp>
#include <libplatform/api.hpp>

namespace UnityBootProtocol {
    struct BootInfo {
        Arch::Type arch;
        
        Platform::FirmwareType firmware;
        Platform::BootloaderType bootloader;
        Platform::HDType hardwareDescriptoinType;

        uintptr_t kernelELFStart;
        uintptr_t kernelELFEnd;

        uintptr_t heapStart;
        uintptr_t heapEnd;
        
        const char* cmdline;
    };
}