#pragma once
#include <cstddef>
#include <cstdint>

namespace Arch::X86::Interrupt {
    //interrupt descriptor table
    struct [[gnu::packed]] IDTEntry {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t ist;
        uint8_t type_attr;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t zero;
    };

    struct [[gnu::packed]] IDTPointer {
        uint16_t limit;
        uint64_t base;
    };

    struct [[gnu::packed]] InterruptFrame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    };

    void setGate(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags);
    void loadIDT(void* idt_ptr, size_t size);
    void init();
    void picRemap();

    void enable();
    void disable();
}