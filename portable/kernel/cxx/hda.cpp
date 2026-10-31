#include <cstdint>

namespace Kernel::HDA {
    struct HDABufferDescriptor {
        uint64_t address;
        uint32_t length;
        uint32_t flags;
    };

    struct alignas(128) HDABDL {
        HDABufferDescriptor entries[2];
    };

    // alignas(128) static HDABDL bdl;
    // alignas(128) static uint8_t pcmBuffer[8192];

    // void prepareDmaBuffers() {
    //     for (uint32_t i = 0; i < sizeof(pcmBuffer); ++i) pcmBuffer[i] = 0;

    //     bdl.entries[0].address = reinterpret_cast<uintptr_t>(pcmBuffer);
    //     bdl.entries[0].length = sizeof(pcmBuffer);
    //     bdl.entries[0].flags = 1;

    //     bdl.entries[1].address = reinterpret_cast<uintptr_t>(pcmBuffer);
    //     bdl.entries[1].length = sizeof(pcmBuffer);
    //     bdl.entries[1].flags = 1;
    // }
}