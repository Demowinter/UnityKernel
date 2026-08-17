#include <cstdint>
#include <libkernel/memory.hpp>
#include <libkstd/string.hpp>
#include <libarch/x86/cpu.hpp>

namespace Arch::X86::CPU {
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
        uint32_t mf[3] = {info.ebx, info.edx, info.ecx};

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