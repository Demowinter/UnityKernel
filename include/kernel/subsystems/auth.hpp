#pragma once
#include <cstdint>

namespace Kernel::AuthSubsystem {
    using UserID = std::uint16_t;
    using GroupID = std::uint16_t;

    struct UserAuthData {
        UserID uid;
        GroupID gid;
    };
}