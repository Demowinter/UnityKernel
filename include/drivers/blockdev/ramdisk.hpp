#pragma once
#include <cstddef>
#include <cstdint>
#include <libstd/vector.hpp>
#include <drivers/blockdev/api.hpp>

namespace Driver::Blockdev {
    constexpr uint32_t sectorSize = 512;

    class RamdiskDriver : public BlockdevDriver {
    public:
        RamdiskDriver();    

        bool initialize() override;
        void finalize() override;

        void resize(uint32_t sectors) override;

        bool read(uint32_t lba, void* buffer, uint32_t count = 1) const override;
        bool write(uint32_t lba, const void* buffer, uint32_t count = 1) override;

        uint32_t sectors() const override;

    private:
        STDLib::Vector<uint8_t> storage;
    };
}
