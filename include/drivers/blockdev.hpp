#pragma once
#include <limits>
#include <cstddef>
#include <cstdint>
#include <libstd/vector.hpp>
#include <drivers/driver.hpp>

namespace Driver::Blockdev {
    using BlockDeviceID = std::size_t;

    constexpr BlockDeviceID invalidID = std::numeric_limits<BlockDeviceID>::max();
    
    constexpr uint32_t sectorSize = 512;
    
    class BlockdevDriver : public BaseDriver {
    public:
        BlockdevDriver(Type driType) : BaseDriver{Category::BLK, driType} {}
        virtual ~BlockdevDriver() = default;

        virtual BlockDeviceID createDevice() { return invalidID; }
        virtual STDLib::Vector<BlockDeviceID> listDevices() { return {}; }

        virtual void resize([[maybe_unused]] BlockDeviceID blkid, [[maybe_unused]] uint32_t sectors) {}

        virtual bool read(BlockDeviceID blkid, uint32_t lba, void* buffer, uint32_t count = 1) const = 0;
        virtual bool write(BlockDeviceID blkid, uint32_t lba, const void* buffer, uint32_t count = 1) = 0;

        virtual uint32_t sectors(BlockDeviceID blkid) const = 0;
    };
}