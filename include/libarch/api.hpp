#pragma once
#include <libkstd/string.hpp>

namespace Arch {
    void initialize();

    namespace CPU {
        KernelSTD::String manufacturer();

        [[noreturn]] void halt();
    }

    namespace Interrupt {
        void enable();
        void disable();
    }
}