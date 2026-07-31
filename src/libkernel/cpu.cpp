#include <array>
#include <cstdint>
#include <libkernel/cpu.hpp>

namespace Kernel::CPU {
    Info cpuid(uint32_t leaf) {
        Info info;

        asm volatile (
            "cpuid"
            : "=a"(info.eax), "=b"(info.ebx), "=c"(info.ecx), "=d"(info.edx)
            : "a"(leaf)
            : "cc"
        );

        return info;
    }

    std::array<uint32_t, 3> manufacturer(const Info& info) {
        std::array<uint32_t, 3> binName;
        binName[0] = info.ebx;
        binName[1] = info.edx;
        binName[2] = info.ecx;

        return binName;
    }

    void interrupts(bool on) {
        if (on) asm("sti");
        else asm("cli");
    }

    void halt() {
        asm("hlt");
    }
}