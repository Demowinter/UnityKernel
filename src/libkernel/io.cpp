#include <cstdint>
#include <libkernel/io.hpp>

namespace Kernel::IO {
    namespace PMIO {
        template<>
        uint8_t read<uint8_t>(uint16_t addr) {
            uint8_t val;

            asm volatile (
                "inb %1, %0"
                : "=a"(val)
                : "d"(addr)
            );

            return val;
        }

        template<>
        bool read<bool>(uint16_t addr) {
            return read<uint8_t>(addr);
        }

        template<>
        uint16_t read<uint16_t>(uint16_t addr) {
            uint16_t val;

            asm volatile (
                "inw %1, %0"
                : "=a"(val)
                : "d"(addr)
            );

            return val;
        }

        template<>
        uint32_t read<uint32_t>(uint16_t addr) {
            uint32_t val;

            asm volatile (
                "inl %1, %0"
                : "=a"(val)
                : "d"(addr)
            );
            
            return val;
        }

        template<typename Type>
        void write(uint16_t addr, Type val);

        template<>
        void write(uint16_t addr, uint8_t val) {
            asm volatile (
                "outb %0, %1"
                :
                : "a"(val), "d"(addr)
            );
        }

        template<>
        void write(uint16_t addr, bool val) {
            write<uint8_t>(addr, val);
        }

        template<>
        void write(uint16_t addr, uint16_t val) {
            asm volatile (
                "outw %0, %1"
                :
                : "a"(val), "d"(addr)
            );
        }

        template<>
        void write(uint16_t addr, uint32_t val) {
            asm volatile (
                "outl %0, %1"
                :
                : "a"(val), "d"(addr)
            );
        }
    }

    namespace MMIO {

    }

    namespace Interrupt {

    }
}