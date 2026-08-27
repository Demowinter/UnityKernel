#pragma once
#include <string_view>

namespace ENV::System {
    [[noreturn]] void panic(std::string_view who, std::string_view what);
}