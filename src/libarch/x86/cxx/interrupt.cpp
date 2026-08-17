#include <cstddef>
#include <libarch/x86/interrupt.hpp>

namespace Arch::X86::Intrrupt {
    //interrupt descriptor table
    void loadIDT(void* idt_ptr, size_t size) {
        asm volatile (
            "lidt (%0)"
            :
            : "r"(idt_ptr)
        );
    }
}