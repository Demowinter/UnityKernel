#pragma once
#include <cstdint>

namespace Arch::X86::PMIO {
    struct Region {
        uint16_t start;
        uint16_t end;
    };

    template<typename Type>
    Type read(uint16_t port);

    template<typename Type>
    void write(uint16_t port, Type val);
}