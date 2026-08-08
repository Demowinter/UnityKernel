#include <cstdint>
#include <libkernel/memory.hpp>
#include <libkernel/cpu.hpp>
#include <libkstd/string.hpp>

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

    KernelSTD::String manufacturer(const Info& info) {
        uint32_t* mf = new uint32_t[3];
        mf[0] = info.ebx;
        mf[1] = info.edx;
        mf[2] = info.ecx;

        return KernelSTD::String(reinterpret_cast<char*>(mf), 12);
    }

    void interrupts(bool on) {
        if (on) asm("sti");
        else asm("cli");
    }

    [[noreturn]] void halt() {
        while (true) asm("hlt");
    }
}