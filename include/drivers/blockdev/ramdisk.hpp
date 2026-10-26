#pragma once
#include <cstdint>
#include <libstd/vector.hpp>
#include <libstd/map.hpp>
#include <kernel/subsystems/resource.hpp>
#include <drivers/blockdev.hpp>

namespace Driver::Blockdev {
    class RamdiskDriver final : public BlockdevDriver {
    public:
        RamdiskDriver();    

        bool initialize() override;
        void finalize() override;

        void resize(Kernel::ResourceSubsystem::ResourceID resid, uint32_t sectors) override;

        bool read(Kernel::ResourceSubsystem::ResourceID resid, uint32_t lba, void* buffer, uint32_t count = 1) const override;
        bool write(Kernel::ResourceSubsystem::ResourceID resid, uint32_t lba, const void* buffer, uint32_t count = 1) override;

        uint32_t sectors(Kernel::ResourceSubsystem::ResourceID resid) const override;

    private:
        STDLib::Map<Kernel::ResourceSubsystem::ResourceID, STDLib::Vector<uint8_t>> resources;
    };
}
