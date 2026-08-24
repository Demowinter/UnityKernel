#include <cstdint>
#include <libgrub/multiboot.hpp>

namespace Kernel::GRUB {
    bool checkMultiboot(uint32_t magic)  {
        return magic == multibootMagic;
    }
}
