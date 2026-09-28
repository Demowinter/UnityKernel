#pragma once
#include <drivers/api.hpp>

namespace Driver::Blockdev {
    constexpr uint32_t sectorSize = 512;
    
    class BlockdevDriver : public BaseDriver {
    public:
        BlockdevDriver(Type driType) : BaseDriver{Category::BLK, driType} {}
        virtual ~BlockdevDriver() = default;

        virtual void resize(uint32_t sectors) {}

        virtual bool read(uint32_t lba, void* buffer, uint32_t count = 1) const = 0;
        virtual bool write(uint32_t lba, const void* buffer, uint32_t count = 1) = 0;

        virtual uint32_t sectors() const = 0;
    };
}