#pragma once
#include <string_view>

namespace UnityBoot::System {
    [[noreturn]] void panic(std::string_view who, std::string_view what);
}