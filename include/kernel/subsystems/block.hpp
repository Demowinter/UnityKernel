#pragma once
#include <limits>
#include <cstddef>

namespace Kernel::BlockSubsystem {
    using ResourceID = std::size_t;

    constexpr ResourceID invalidID = std::numeric_limits<ResourceID>::max();
}