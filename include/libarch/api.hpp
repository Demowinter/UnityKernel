#pragma once
#include <libstd/string.hpp>

namespace Arch {
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