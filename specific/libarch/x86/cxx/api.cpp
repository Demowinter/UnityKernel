#include <libkstd/string.hpp>
#include <libarch/x86/cpu.hpp>
#include <libarch/x86/interrupt.hpp>
#include <libarch/api.hpp>

namespace Arch {
    void initialize() {
        X86::Interrupt::init();
    }

    namespace CPU {
        KernelSTD::String manufacturer() {
            return X86::CPU::manufacturer(X86::CPU::cpuid(0));
        }

        [[noreturn]] void halt() {
            X86::CPU::halt();
        }
    }

    namespace Interrupt {
        void enable() {
            X86::Interrupt::enable();
        }

        void disable() {
            X86::Interrupt::disable();
        }
    }
}