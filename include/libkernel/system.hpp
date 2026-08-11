#pragma once
#include <string_view>

namespace Kernel::System {
    [[noreturn]] void panic(std::string_view who, std::string_view what);
}