#pragma once
#include <limits>
#include <cstdint>

namespace Kernel::AuthSubsystem {
    using UserID = std::uint16_t;
    using GroupID = std::uint16_t;

    constexpr UserID invalidUID = std::numeric_limits<UserID>::max();
    constexpr GroupID invalidGID = std::numeric_limits<GroupID>::max();

    struct UserAuthData {
        UserID uid;
        GroupID gid;
    };
}