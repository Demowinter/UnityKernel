#pragma once
#include <cstdint>

namespace Arch::X86::IO {
    namespace PMIO {
        template<typename Type>
        Type read(uint16_t port);

        template<>
        uint8_t read<uint8_t>(uint16_t port);

        template<>
        bool read<bool>(uint16_t port);

        template<>
        uint16_t read<uint16_t>(uint16_t port);

        template<>
        uint32_t read<uint32_t>(uint16_t port);
        

        template<typename Type>
        void write(uint16_t port, Type val);

        template<>
        void write(uint16_t port, uint8_t val);

        template<>
        void write(uint16_t port, bool val);

        template<>
        void write(uint16_t port, uint16_t val);

        template<>
        void write(uint16_t port, uint32_t val);
    }

    namespace MMIO {

    }

    namespace Interrupt {
        //interrupt descriptor table
        void loadIDT(void* idt_ptr, size_t size);
    }
}