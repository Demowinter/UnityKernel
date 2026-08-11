#pragma once
#include <string_view>

namespace KernelRT {
    void initialize();
    void finalize();
    
    [[noreturn]] void abort(std::string_view what);
}