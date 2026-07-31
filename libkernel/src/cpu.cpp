#include <cstdint>
#include <cpu.hpp>

namespace Kernel::CPU {
    Info cpuid(uint32_t leaf) {
        Info info = {.eax = leaf};

        asm (
            "cpuid"
            : "+a"(info.eax), "=b"(info.ebx), "=c"(info.ecx), "=d"(info.edx)
            :
            : "cc"
        );

        return info;
    }

    void interrupts(bool on) {
        if (on) asm("sti");
        else asm("cli");
    }

    void halt() {
        asm("hlt");
    }
}