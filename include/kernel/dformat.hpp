#pragma once
#include <cstdint>
#include <drivers/blockdev/api.hpp>

namespace Kernel::DFORMAT {
    enum class Status {
        ok,
        invalidDevice,
        diskTooSmall,
        unsupportedCapacity,
        ioError
    };

    Status partitionAndFormatFat32(Driver::Blockdev::BlockdevDriver& device);
    const char* statusText(Status status);
}