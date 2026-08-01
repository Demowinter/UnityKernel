#include <cstdint>
#include <kernel32/grub.hpp>

namespace Kernel::GRUB {
    struct [[gnu::packed]] Tag {
        uint32_t type;
        uint32_t size;
    };

    bool checkMultiboot(uint32_t magic)  {
        return magic == multibootMagic;
    }

    const Framebuffer* findFramebuffer(uint32_t multibootInfoAddress) {
        const auto* info = reinterpret_cast<const uint8_t*>(multibootInfoAddress);
        const uint32_t totalSize = *reinterpret_cast<const uint32_t*>(info);

        if (totalSize < 16) {
            return nullptr;
        }

        const auto* end = info + totalSize;
        auto* tag = reinterpret_cast<const Tag*>(info + 8);

        while (reinterpret_cast<const uint8_t*>(tag) + sizeof(Tag) <= end) {
            if (tag->type == 0) {
                return nullptr;
            }

            if (tag->size < sizeof(Tag) ||
                reinterpret_cast<const uint8_t*>(tag) + tag->size > end) {
                return nullptr;
            }

            if (tag->type == 8 && tag->size >= sizeof(Framebuffer)) {
                return reinterpret_cast<const Framebuffer*>(tag);
            }

            const auto next = (reinterpret_cast<uintptr_t>(tag) + tag->size + 7) & ~uintptr_t(7);
            tag = reinterpret_cast<const Tag*>(next);
        }

        return nullptr;
    }
}
