#include <cstddef>
#include <cstdint>
#include <libarch/x86/mmio.hpp>
#include <libarch/x86/pmio.hpp>
#include <libarch/x86/interrupt.hpp>

namespace Arch::X86::Interrupt {
    static IDTEntry entries[256];

    void setGate(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags) {
        entries[num] = {
            .offset_low = static_cast<uint16_t>(handler & 0xFFFF),
            .selector = sel,
            .ist = 0,
            .type_attr = flags,
            .offset_mid = static_cast<uint16_t>((handler >> 16) & 0xFFFF),
            .offset_high = static_cast<uint32_t>((handler >> 32) & 0xFFFFFFFF),
            .zero = 0
        };
    }

    void loadIDT(void* idt_ptr, size_t size) {
        IDTPointer idt_pointer { static_cast<uint16_t>(size - 1), reinterpret_cast<uint64_t>(idt_ptr) };
        asm volatile("lidt %0" : : "m"(idt_pointer));
    }

    void picRemap() {
        using namespace Arch::X86::PMIO;
        write<uint8_t>(0x20, 0x11);
        write<uint8_t>(0xA0, 0x11);
        write<uint8_t>(0x21, 0x20); // master -> 32-39
        write<uint8_t>(0xA1, 0x28); // slave  -> 40-47
        write<uint8_t>(0x21, 0x04);
        write<uint8_t>(0xA1, 0x02);
        write<uint8_t>(0x21, 0x01);
        write<uint8_t>(0xA1, 0x01);
        write<uint8_t>(0x21, 0x0);
        write<uint8_t>(0xA1, 0x0);
    }

    // --Handlers--
    __attribute__((interrupt))
    static void isr_div0(InterruptFrame*) {
        asm volatile("cli; hlt");
    }

    __attribute__((interrupt))
    static void isr_breakpoint(InterruptFrame*) {
        asm volatile("cli; hlt");
    }

    __attribute__((interrupt))
    static void isr_gpf(InterruptFrame*, unsigned int error_code) {
        asm volatile("cli; hlt");
    }

    __attribute__((interrupt))
    static void isr_page_fault(InterruptFrame*, unsigned int error_code) {
        uint32_t faultAddr;
        asm volatile("movl %%cr2, %0" : "=r"(faultAddr));
        (void)faultAddr; // Mark as intentionally used for fault handling
        asm volatile("cli; hlt");
    }

    __attribute__((interrupt))
    static void irq_timer(InterruptFrame*) {
        // Handle timer interrupt
        Arch::X86::PMIO::write<uint8_t>(0x20, 0x20); // send EOI to master PIC
    }

    void init() {
        picRemap();
        setGate(0, reinterpret_cast<uint64_t>(isr_div0), 0x08, 0x8E);
        setGate(3, reinterpret_cast<uint64_t>(isr_breakpoint), 0x08, 0x8E);
        setGate(13, reinterpret_cast<uint64_t>(isr_gpf), 0x08, 0x8E);
        setGate(14, reinterpret_cast<uint64_t>(isr_page_fault), 0x08, 0x8E);
        setGate(32, reinterpret_cast<uint64_t>(irq_timer), 0x08, 0x8E);

        loadIDT(entries, sizeof(entries));
        asm volatile("sti");
    }

    void enable() {
        asm volatile("sti");
    }

    void disable() {
        asm volatile("cli");
    }
}