#pragma once
#include <cstddef>
#include <cstdint>

namespace Kernel::PCISubsystem {
    struct Device {
        uint32_t bus;
        uint32_t device;
        uint32_t function;
    };
}