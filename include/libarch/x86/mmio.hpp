#pragma once
#include <cstdint>

namespace Arch::X86::MMIO {
    struct Region {
        uintptr_t start;
        uintptr_t end;
    };

    template<typename T>
    inline T read(uintptr_t addr) {
        return *reinterpret_cast<volatile T*>(addr);
    }

    template<typename T>
    inline void write(uintptr_t addr, T val) {
        *reinterpret_cast<volatile T*>(addr) = val;
    }

    template<typename T>
    class Register {
        volatile T value;

    public:
        Register() : value(0) {}
        explicit Register(T initial) : value(initial) {}

        T read() const {
            return value;
        }

        void write(T val) {
            value = val;
        }

        void setBits(T mask) {
            value |= mask;
        }

        void clearBits(T mask) {
            value &= ~mask;
        }

        bool testBits(unsigned n) const {
            return (value & (1U << n)) != 0;
        }

        bool testMask(T mask) const {
            return (value & mask) != 0;
        }

        T getMask(T mask) const {
            return value & mask;
        }

        // Operator overloads for convenience
        operator T() const {
            return read();
        }

        Register& operator=(T val) {
            write(val);
            return *this;
        }

        Register& operator|=(T mask) {
            setBits(mask);
            return *this;
        }

        Register& operator&=(T mask) {
            clearBits(~mask);
            return *this;
        }
    };

    inline void compiler_barrier() { 
        asm volatile("" ::: "memory"); 
    }

    inline void mfence() { 
        asm volatile("mfence" ::: "memory"); 
    }

    inline void lfence() {
        asm volatile("lfence" ::: "memory");
    }

    inline void sfence() {
        asm volatile("sfence" ::: "memory");
    }

    // Wait for register condition with timeout
    template<typename T>
    bool waitUntil(volatile T& reg, T mask, T expected, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == expected) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    // Wait while condition is true
    template<typename T>
    bool waitWhile(volatile T& reg, T mask, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == 0) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }
}