#include <cstdint>
#include <kernel/grub.hpp>

namespace Kernel::GRUB {
    bool checkMultiboot(uint32_t magic)  {
        return magic == multibootMagic;
    }
}
