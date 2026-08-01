#pragma once
#include <cstdint>

namespace Kernel::GRUB {
    constexpr uint32_t multibootMagic = 0x36D76289;

    struct Multiboot {

    };

    bool checkMultiboot(uint32_t magic);
}