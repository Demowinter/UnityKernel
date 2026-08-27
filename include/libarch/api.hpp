#pragma once
#include <libstd/string.hpp>

namespace Arch {
    void initialize();

    namespace CPU {
        STDLib::String manufacturer();

        [[noreturn]] void halt();
    }

    namespace Interrupt {
        void enable();
        void disable();
    }
}