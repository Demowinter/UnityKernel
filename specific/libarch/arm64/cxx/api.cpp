#include <libkstd/string.hpp>
#include <libarch/api.hpp>

namespace Arch {
    void initialize() {

    }

    namespace CPU {
        KernelSTD::String manufacturer() {
            return {};
        }

        [[noreturn]] void halt() {
            while (true) {}
        }
    }

    namespace Interrupt {
        void enable() {

        }

        void disable() {

        }
    }
}