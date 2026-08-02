#include <libkrt/krt.hpp>

namespace KernelRT {
    using ConstructorFunc = void(*)();

    extern "C" {
        extern ConstructorFunc __init_array_start[];
        extern ConstructorFunc __init_array_end[];
    }

    void initialize() {
        for (auto ctor = __init_array_start; ctor != __init_array_end; ctor++) (*ctor)();
    }
}