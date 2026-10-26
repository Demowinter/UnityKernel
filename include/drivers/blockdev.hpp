#pragma once
#include <kernel/subsystems/resource.hpp>
#include <drivers/driver.hpp>

namespace Driver::Blockdev {
    constexpr uint32_t sectorSize = 512;
    
    class BlockdevDriver : public BaseDriver {
    public:
        BlockdevDriver(Type driType) : BaseDriver{Category::BLK, driType} {}
        virtual ~BlockdevDriver() = default;

        virtual void resize([[maybe_unused]] Kernel::ResourceSubsystem::ResourceID resid, [[maybe_unused]] uint32_t sectors) {}

        virtual bool read(Kernel::ResourceSubsystem::ResourceID resid, uint32_t lba, void* buffer, uint32_t count = 1) const = 0;
        virtual bool write(Kernel::ResourceSubsystem::ResourceID resid, uint32_t lba, const void* buffer, uint32_t count = 1) = 0;

        virtual uint32_t sectors(Kernel::ResourceSubsystem::ResourceID resid) const = 0;
    };
}