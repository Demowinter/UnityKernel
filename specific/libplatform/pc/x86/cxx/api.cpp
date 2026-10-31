#include <libplatform/platform.hpp>

namespace Platform {
    Info platformInfo{};

    bool initialize(const Info& info) {
        platformInfo = info;

        return true;
    }
}