#include <cstddef>
#pragma once

namespace Arch::X86::Interrupt {
    //interrupt descriptor table
    void loadIDT(void* idt_ptr, size_t size);

    void enable();
    void disable();
}