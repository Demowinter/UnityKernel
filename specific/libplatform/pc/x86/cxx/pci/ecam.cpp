#include <optional>
#include <cstdint>
#include <libarch/x86/pmio.hpp>
#include <libplatform/pc/x86/pci/ecam.hpp>

namespace Platform::PCI::ECAM {
    // uint32_t getAddress(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
    //     return (1U << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    // }

    template<>
    std::optional<uint8_t> readConfig<uint8_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return std::nullopt;
    }
    
    template<>
    std::optional<uint16_t> readConfig<uint16_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return std::nullopt;
    }

    template<>
    std::optional<uint32_t> readConfig<uint32_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return std::nullopt;
    }

    template<>
    bool writeConfig<uint8_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint8_t value) {
        return false;
    }

    template<>
    bool writeConfig<uint16_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint16_t value) {
        return false;
    }

    template<>
    bool writeConfig<uint32_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value) {
        return false;
    }

    bool isAvailable() {
        // for (uint32_t bus = 0; bus < 256; bus++) {
        //     for (uint32_t device = 0; device < 32; device++) {
        //         auto vendor = Platform::PCI::CM1::readConfig<uint16_t>(bus, device, 0, 0x00);

        //         if (!vendor || *vendor == 0xFFFF || *vendor == 0x0000) continue;

        //         return true;
        //     }
        // }

        return false;
    }
}