#include <algorithm>
#include <libbase/memory.hpp>
#include <drivers/blockdev/ramdisk.hpp>

namespace Driver::Blockdev {
    RamdiskDriver::RamdiskDriver() : BlockdevDriver{Type::RAMDISK} {}

    bool RamdiskDriver::initialize() { return true; }
    void RamdiskDriver::finalize() {}

    void RamdiskDriver::resize(uint32_t sectors) {
        storage.resize(sectors * sectorSize);
    }

    bool RamdiskDriver::read(uint32_t lba, void* buffer, uint32_t count) const {
        if (!storage.size() || !buffer) return false;
        if (lba > sectors() || count > sectors() - lba) return false;

        auto it = storage.begin() + lba * sectorSize;

        uint8_t* bytebuf = static_cast<uint8_t*>(buffer);

        std::copy(it, it + count * sectorSize, bytebuf);

        // memcpy(buffer, storage + lba * sectorSize, count * sectorSize);

        return true;
    }

    bool RamdiskDriver::write(uint32_t lba, const void* buffer, uint32_t count) {
        if (!storage.size() || !buffer) return false;
        if (lba > sectors() || count > sectors() - lba) return false;

        auto it = storage.begin() + lba * sectorSize;

        const uint8_t* bytebuf = static_cast<const uint8_t*>(buffer);

        std::copy(bytebuf, bytebuf + count * sectorSize, it);

        // memcpy(storage + lba * sectorSize, buffer, count * sectorSize);

        return true;
    }

    uint32_t RamdiskDriver::sectors() const {
        return storage.size() / sectorSize;
    }
}
