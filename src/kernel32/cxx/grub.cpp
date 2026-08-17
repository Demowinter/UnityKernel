#include <cstdint>
#include <kernel32/grub.hpp>

namespace Kernel::GRUB {
    bool checkMultiboot(uint32_t magic)  {
        return magic == multibootMagic;
    }
}