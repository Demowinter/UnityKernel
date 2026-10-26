#include <algorithm>
#include <libbase/memory.hpp>
#include <kernel/subsystems/resource.hpp>
#include <drivers/blockdev/ramdisk.hpp>

namespace Driver::Blockdev {
    RamdiskDriver::RamdiskDriver() : BlockdevDriver{Type::RAMDISK} {}

    bool RamdiskDriver::initialize() {
        Kernel::ResourceSubsystem::ResourceInfo resource{};
        resource.type = Kernel::ResourceSubsystem::ResourceType::SOMETHING_ELSE;
        resource.creator = this;
        resource.maintaner = this;

        resources.insert(Kernel::ResourceSubsystem::registerResource(resource), {});

        return true;
    }

    void RamdiskDriver::finalize() {
        for (auto it = resources.begin(); it != resources.end(); it++) Kernel::ResourceSubsystem::unregisterResource(it.key());
    }

    void RamdiskDriver::resize(Kernel::ResourceSubsystem::ResourceID resid, uint32_t sectors) {
        resources.at(resid).resize(sectors * sectorSize);
    }

    bool RamdiskDriver::read(Kernel::ResourceSubsystem::ResourceID resid, uint32_t lba, void* buffer, uint32_t count) const {
        if (!resources.at(resid).size() || !buffer) return false;
        if (lba > sectors(resid) || count > sectors(resid) - lba) return false;

        auto it = resources.at(resid).begin() + lba * sectorSize;

        uint8_t* bytebuf = static_cast<uint8_t*>(buffer);

        std::copy(it, it + count * sectorSize, bytebuf);

        // memcpy(buffer, storage + lba * sectorSize, count * sectorSize);

        return true;
    }

    bool RamdiskDriver::write(Kernel::ResourceSubsystem::ResourceID resid, uint32_t lba, const void* buffer, uint32_t count) {
        if (!resources.at(resid).size() || !buffer) return false;
        if (lba > sectors(resid) || count > sectors(resid) - lba) return false;

        auto it = resources.at(resid).begin() + lba * sectorSize;

        const uint8_t* bytebuf = static_cast<const uint8_t*>(buffer);

        std::copy(bytebuf, bytebuf + count * sectorSize, it);

        // memcpy(storage + lba * sectorSize, buffer, count * sectorSize);

        return true;
    }

    uint32_t RamdiskDriver::sectors(Kernel::ResourceSubsystem::ResourceID resid) const {
        return resources.at(resid).size() / sectorSize;
    }
}
