#include <cstdint>
#include <libenv/memory.hpp>
#include <libstd/string.hpp>
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

    STDLib::String manufacturer(const Info& info) {
        uint32_t mf[3] = {info.ebx, info.edx, info.ecx};

        return STDLib::String(reinterpret_cast<char*>(mf), 12);
    }

    [[noreturn]] void halt() {
        while (true) asm("hlt");
    }
}