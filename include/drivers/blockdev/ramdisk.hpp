#pragma once
#include <cstdint>
#include <libstd/vector.hpp>
#include <libstd/map.hpp>
#include <kernel/subsystems/block.hpp>
#include <drivers/blockdev.hpp>

namespace Driver::Blockdev {
    class RamdiskDriver final : public BlockdevDriver {
    public:
        RamdiskDriver();    

        bool initialize() override;
        void finalize() override;

        void resize(Kernel::BlockSubsystem::ResourceID resid, uint32_t sectors) override;

        bool read(Kernel::BlockSubsystem::ResourceID resid, uint32_t lba, void* buffer, uint32_t count = 1) const override;
        bool write(Kernel::BlockSubsystem::ResourceID resid, uint32_t lba, const void* buffer, uint32_t count = 1) override;

        uint32_t sectors(Kernel::BlockSubsystem::ResourceID resid) const override;

    private:
        STDLib::Map<Kernel::BlockSubsystem::ResourceID, STDLib::Vector<uint8_t>> resources;
    };
}
