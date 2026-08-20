#pragma once
#include <cstdint>

namespace Kernel::GRUB {
    constexpr uint32_t multibootMagic = 0x36D76289;

    struct MultibootInfo {
        uint32_t totalSize;
        uint32_t reserved;
    };

    struct MultibootTag {
        uint32_t type;
        uint32_t size;
    };

    bool checkMultiboot(uint32_t magic);
}