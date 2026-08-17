#pragma once
#include <libkstd/string.hpp>

namespace Arch {
    void initialize();

    namespace CPU {
        KernelSTD::String manufacturer();

        void interrupts(bool on);
        bool interrupts();

        [[noreturn]] void halt();
    }
}