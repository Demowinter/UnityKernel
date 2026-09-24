#pragma once
#include <cstddef>
#include <cstdint>

namespace Kernel::Block {
    constexpr uint32_t sectorSize = 512;

    class MemoryDevice {
    public:
        bool initialize(uint8_t* storage, uint32_t sectorCount);

        bool read(uint32_t lba, void* buffer, uint32_t count = 1) const;
        bool write(uint32_t lba, const void* buffer, uint32_t count = 1);

        uint32_t sectors() const;

    private:
        uint8_t* storage = nullptr;
        uint32_t sectorCount = 0;
    };
}
