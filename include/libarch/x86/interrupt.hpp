#pragma once
#include <cstddef>
#include <cstdint>

namespace Arch::X86::Interrupt {
    //interrupt descriptor table
    struct [[gnu::packed]] IDTEntry {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t zero;
        uint8_t type_attr;
        uint16_t offset_high;
    };

    struct [[gnu::packed]] IDTPointer {
        uint16_t limit;
        uint32_t base;
    };

    struct [[gnu::packed]] InterruptFrame {
        uint32_t eip;
        uint32_t cs;
        uint32_t eflags;
        uint32_t esp;
        uint32_t ss;
    };

    void setGate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags);
    void loadIDT(void* idt_ptr, size_t size);
    void init();
    void picRemap();

    void enable();
    void disable();
}
