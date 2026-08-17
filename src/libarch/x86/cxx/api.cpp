#include <libkstd/string.hpp>
#include <libarch/x86/cpu.hpp>
#include <libarch/api.hpp>

namespace Arch {
    namespace CPU {
        KernelSTD::String manufacturer() {
            return X86::CPU::manufacturer(X86::CPU::cpuid(0));
        }

        void interrupts(bool on) {
            X86::CPU::interrupts(on);
        }

        bool interrupts() {
            return true; // TODO: add interrupts checking logic
        }

        [[noreturn]] void halt() {
            X86::CPU::halt();
        }
    }
}