#pragma once
#include <string_view>

namespace CXXRuntime {
    void initialize();
    void finalize();

    [[noreturn]] void abort(std::string_view what) noexcept;
    [[noreturn]] void abort(std::string_view who, std::string_view what) noexcept;
}