#include <cstddef>
#include <cstdint>
#include <libarch/x86/io.hpp>

namespace Arch::X86::IO {
    namespace PMIO {
        template<>
        uint8_t read<uint8_t>(uint16_t port) {
            uint8_t val;

            asm volatile (
                "inb %1, %0"
                : "=a"(val)
                : "d"(port)
            );

            return val;
        }

        template<>
        bool read<bool>(uint16_t port) {
            return read<uint8_t>(port);
        }

        template<>
        uint16_t read<uint16_t>(uint16_t port) {
            uint16_t val;

            asm volatile (
                "inw %1, %0"
                : "=a"(val)
                : "d"(port)
            );

            return val;
        }

        template<>
        uint32_t read<uint32_t>(uint16_t port) {
            uint32_t val;

            asm volatile (
                "inl %1, %0"
                : "=a"(val)
                : "d"(port)
            );
            
            return val;
        }

        template<typename Type>
        void write(uint16_t port, Type val);

        template<>
        void write(uint16_t port, uint8_t val) {
            asm volatile (
                "outb %0, %1"
                :
                : "a"(val), "d"(port)
            );
        }

        template<>
        void write(uint16_t port, bool val) {
            write<uint8_t>(port, val);
        }

        template<>
        void write(uint16_t port, uint16_t val) {
            asm volatile (
                "outw %0, %1"
                :
                : "a"(val), "d"(port)
            );
        }

        template<>
        void write(uint16_t port, uint32_t val) {
            asm volatile (
                "outl %0, %1"
                :
                : "a"(val), "d"(port)
            );
        }
    }

    namespace MMIO {

    }

    namespace Interrupt {
        //interrupt descriptor table
        void loadIDT(void* idt_ptr, size_t size) {
            asm volatile (
                "lidt (%0)"
                :
                : "r"(idt_ptr)
            );
        }
    }
}