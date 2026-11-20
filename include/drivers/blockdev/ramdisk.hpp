#pragma once
#include <cstdint>
#include <libstd/vector.hpp>
#include <libstd/map.hpp>
#include <drivers/blockdev.hpp>

namespace Driver::Blockdev {
    class RamdiskDriver final : public BlockdevDriver {
    public:
        RamdiskDriver();    

        bool initialize() override;
        void finalize() override;

        BlockDeviceID createDevice() override;
        STDLib::Vector<BlockDeviceID> listDevices() override;

        void resize(BlockDeviceID blkid, uint32_t sectors) override;

        bool read(BlockDeviceID blkid, uint32_t lba, void* buffer, uint32_t count = 1) const override;
        bool write(BlockDeviceID blkid, uint32_t lba, const void* buffer, uint32_t count = 1) override;

        uint32_t sectors(BlockDeviceID blkid) const override;

    private:
        STDLib::Map<BlockDeviceID, STDLib::Vector<uint8_t>> devices;
    };
}
