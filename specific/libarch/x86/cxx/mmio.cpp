#include <cstdint>
#include <libarch/x86/mmio.hpp>

namespace Arch::X86::MMIO {
    //Explicit template instantiation for read()
    template<>
    uint8_t read<uint8_t>(uintptr_t addr) {
        return *reinterpret_cast<volatile uint8_t*>(addr);
    }

    template<>
    uint16_t read<uint16_t>(uintptr_t addr) {
        return *reinterpret_cast<volatile uint16_t*>(addr);
    }

    template<>
    uint32_t read<uint32_t>(uintptr_t addr) {
        return *reinterpret_cast<volatile uint32_t*>(addr);
    }

    template<>
    uint64_t read<uint64_t>(uintptr_t addr) {
        return *reinterpret_cast<volatile uint64_t*>(addr);
    }

    //Explicit template instantiation for write()
    template<>
    void write<uint8_t>(uintptr_t addr, uint8_t val) {
        *reinterpret_cast<volatile uint8_t*>(addr) = val;
    }

    template<>
    void write<uint16_t>(uintptr_t addr, uint16_t val) {
        *reinterpret_cast<volatile uint16_t*>(addr) = val;
    }

    template<>
    void write<uint32_t>(uintptr_t addr, uint32_t val) {
        *reinterpret_cast<volatile uint32_t*>(addr) = val;
    }

    template<>
    void write<uint64_t>(uintptr_t addr, uint64_t val) {
        *reinterpret_cast<volatile uint64_t*>(addr) = val;
    }

    //Explicit template instantiation for Register class
    template class Register<uint8_t>;
    template class Register<uint16_t>;
    template class Register<uint32_t>;
    template class Register<uint64_t>;

    //Explicit template instantiation for waitUntil()
    template<>
    bool waitUntil<uint8_t>(volatile uint8_t& reg, uint8_t mask, uint8_t expected, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == expected) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    template<>
    bool waitUntil<uint16_t>(volatile uint16_t& reg, uint16_t mask, uint16_t expected, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == expected) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    template<>
    bool waitUntil<uint32_t>(volatile uint32_t& reg, uint32_t mask, uint32_t expected, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == expected) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    template<>
    bool waitUntil<uint64_t>(volatile uint64_t& reg, uint64_t mask, uint64_t expected, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == expected) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    // Explicit template instantiation for waitWhile()
    template<>
    bool waitWhile<uint8_t>(volatile uint8_t& reg, uint8_t mask, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == 0) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    template<>
    bool waitWhile<uint16_t>(volatile uint16_t& reg, uint16_t mask, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == 0) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    template<>
    bool waitWhile<uint32_t>(volatile uint32_t& reg, uint32_t mask, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == 0) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }

    template<>
    bool waitWhile<uint64_t>(volatile uint64_t& reg, uint64_t mask, uint64_t max_iters) {
        for (uint64_t i = 0; i < max_iters; ++i) {
            if ((reg & mask) == 0) {
                return true;
            }
            compiler_barrier();
        }
        return false;
    }
}