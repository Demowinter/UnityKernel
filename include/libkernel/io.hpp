#pragma once
#include <cstdint>

namespace Kernel::IO {
    namespace PMIO {
        template<typename Type>
        Type read(uint16_t addr);

        template<>
        uint8_t read<uint8_t>(uint16_t addr);

        template<>
        bool read<bool>(uint16_t addr);

        template<>
        uint16_t read<uint16_t>(uint16_t addr);

        template<>
        uint32_t read<uint32_t>(uint16_t addr);
        

        template<typename Type>
        void write(uint16_t addr, Type val);

        template<>
        void write(uint16_t addr, uint8_t val);

        template<>
        void write(uint16_t addr, bool val);

        template<>
        void write(uint16_t addr, uint16_t val);

        template<>
        void write(uint16_t addr, uint32_t val);
    }

    namespace MMIO {

    }

    namespace Interrupt {

    }
}