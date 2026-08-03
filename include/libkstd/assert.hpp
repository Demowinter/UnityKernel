#pragma once
#include <string_view>

namespace KernelSTD {
    void assert(bool condition, std::string_view name);
}