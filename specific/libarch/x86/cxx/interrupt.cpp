#include <array>
#include <cstddef>
#include <cstdint>
#include <libarch/x86/cpu.hpp>
#include <libarch/x86/pmio.hpp>
#include <libarch/x86/mmio.hpp>
#include <libarch/x86/interrupt.hpp>
#include <libenv/tty.hpp>

namespace Arch::X86::Interrupt {
    static std::array<IDTEntry, 256> entries;

    // Keyboard state
    static bool keyboardShift = false;
    static bool keyboardCtrl = false;
    static bool keyboardAlt = false;

    // Scancode constants
    constexpr uint8_t SCANCODE_LSHIFT = 0x2A;
    constexpr uint8_t SCANCODE_RSHIFT = 0x36;
    constexpr uint8_t SCANCODE_LCTRL = 0x1D;
    constexpr uint8_t SCANCODE_LALT = 0x38;
    constexpr uint8_t SCANCODE_CAPSLOCK = 0x3A;
    constexpr uint8_t SCANCODE_RELEASE = 0x80;

    void enableIRQ(uint8_t irq) {
        using namespace Arch::X86::PMIO;
        uint16_t port = (irq < 8) ? 0x21 : 0xA1;
        uint8_t mask = read<uint8_t>(port);
        mask &= ~(1 << (irq % 8));
        write<uint8_t>(port, mask);
    }

    void sendEOI(uint8_t irq) {
        using namespace Arch::X86::PMIO;
        write<uint8_t>(0x20, 0x20);  // Send EOI to master PIC
        if (irq >= 8) {
            write<uint8_t>(0xA0, 0x20);  // Send EOI to slave PIC
        }
    }

    void setGate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags) {
        entries[num] = {
            .offset_low = static_cast<uint16_t>(handler & 0xFFFF),
            .selector = sel,
            .zero = 0,
            .type_attr = flags,
            .offset_high = static_cast<uint16_t>((handler >> 16) & 0xFFFF)
        };
    }

    void loadIDT(void* idt_ptr, size_t size) {
        IDTPointer idt_pointer { static_cast<uint16_t>(size - 1), reinterpret_cast<uint32_t>(idt_ptr) };
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
        //Do not dispatch a hardware interrupt before its driver has registered
        //a valid IDT entry and explicitly unmasks the corresponding IRQ.
        write<uint8_t>(0x21, 0xFF);
        write<uint8_t>(0xA1, 0xFF);
    }

    // --Handlers--
    [[gnu::interrupt]]
    static void isr_div0(InterruptFrame*) {
        disable();
        CPU::halt();
    }

    [[gnu::interrupt]]
    static void isr_breakpoint(InterruptFrame*) {
        disable();
        CPU::halt();
    }

    [[gnu::interrupt]]
    static void isr_gpf(InterruptFrame*, unsigned int error_code) {
        disable();
        CPU::halt();
    }

    [[gnu::interrupt]]
    static void isr_page_fault(InterruptFrame*, unsigned int error_code) {
        [[maybe_unused]] uint32_t faultAddr;

        asm volatile(
            "movl %%cr2, %0"
            : "=r"(faultAddr)
        );

        disable();
        CPU::halt();
    }

    [[gnu::interrupt]]
    static void irq_timer(InterruptFrame*) {
        // Handle timer interrupt
        sendEOI(0);
    }

    [[gnu::interrupt]]
    static void irq_keyboard(InterruptFrame*) {
        using namespace Arch::X86::PMIO;

        // Read scancode from keyboard controller
        uint8_t scancode = read<uint8_t>(0x60);

        // Check for key release
        bool isRelease = (scancode & SCANCODE_RELEASE) != 0;
        scancode &= 0x7F;  // Remove release bit

        // Handle modifier keys
        if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
            keyboardShift = !isRelease;
        } else if (scancode == SCANCODE_LCTRL) {
            keyboardCtrl = !isRelease;
        } else if (scancode == SCANCODE_LALT) {
            keyboardAlt = !isRelease;
        }

        // Send keystroke to console if not a release
        if (!isRelease && scancode != SCANCODE_LSHIFT && scancode != SCANCODE_RSHIFT &&
            scancode != SCANCODE_LCTRL && scancode != SCANCODE_LALT &&
            scancode != SCANCODE_CAPSLOCK) {
            ENV::TTY::onKeyPress(scancode, keyboardShift, keyboardCtrl, keyboardAlt);
        }

        sendEOI(1);
    }

    void init() {
        picRemap();
        setGate(0, reinterpret_cast<uint32_t>(isr_div0), 0x08, 0x8E);
        setGate(3, reinterpret_cast<uint32_t>(isr_breakpoint), 0x08, 0x8E);
        setGate(13, reinterpret_cast<uint32_t>(isr_gpf), 0x08, 0x8E);
        setGate(14, reinterpret_cast<uint32_t>(isr_page_fault), 0x08, 0x8E);
        setGate(32, reinterpret_cast<uint32_t>(irq_timer), 0x08, 0x8E);
        setGate(33, reinterpret_cast<uint32_t>(irq_keyboard), 0x08, 0x8E);

        loadIDT(entries.data(), sizeof(entries));

        // Enable keyboard interrupt (IRQ 1)
        enableIRQ(1);

        enable();
    }

    void enable() {
        asm volatile("sti");
    }

    void disable() {
        asm volatile("cli");
    }
}
