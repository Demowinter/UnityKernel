#include <cstdint>
#include <libgrub/multiboot.hpp>

namespace GRUB {
    bool checkMultiboot(uint32_t magic)  {
        return magic == multibootMagic;
    }
}
