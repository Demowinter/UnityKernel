#pragma once
#include <drivers/api.hpp>

namespace Driver::Blockdev {
    class BlockdevDriver : public BaseDriver {
    public:
        BlockdevDriver(Category driCategory, Type driType, std::initializer_list<Info> deps = {}) : BaseDriver{driCategory, driType, deps} {}
        virtual ~BlockdevDriver() = default;

        virtual void resize(uint32_t sectors) = 0;

        virtual bool read(uint32_t lba, void* buffer, uint32_t count = 1) const = 0;
        virtual bool write(uint32_t lba, const void* buffer, uint32_t count = 1) = 0;

        virtual uint32_t sectors() const;
    };
}