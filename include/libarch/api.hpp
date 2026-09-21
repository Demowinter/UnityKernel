#pragma once
#include <libstd/string.hpp>

namespace Arch {
    enum class Type {
        x86,
        x86_64,
        ARM32,
        ARM64
    };

    void initialize();

    namespace CPU {
        STDLib::String manufacturer();

        void halt();
    }

    namespace Interrupt {
        void enable();
        void disable();
    }
}