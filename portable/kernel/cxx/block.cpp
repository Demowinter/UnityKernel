#include <kernel/block.hpp>
#include <libbase/memory.hpp>

namespace Kernel::Block {
    bool MemoryDevice::initialize(uint8_t* storage, uint32_t sectorCount) {
        if (!storage || sectorCount == 0) return false;

        this->storage = storage;
        this->sectorCount = sectorCount;

        return true;
    }

    bool MemoryDevice::read(uint32_t lba, void* buffer, uint32_t count) const {
        if (!storage || !buffer) return false;
        if (lba > sectorCount || count > sectorCount - lba) return false;

        memcpy(buffer, storage + lba * sectorSize, count * sectorSize);

        return true;
    }

    bool MemoryDevice::write(uint32_t lba, const void* buffer, uint32_t count) {
        if (!storage || !buffer) return false;
        if (lba > sectorCount || count > sectorCount - lba) return false;

        memcpy(storage + lba * sectorSize, buffer, count * sectorSize);

        return true;
    }

    uint32_t MemoryDevice::sectors() const {
        return sectorCount;
    }
}
