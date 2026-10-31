#include <optional>
#include <cstdint>
#include <libarch/x86/pmio.hpp>
#include <libplatform/pci.hpp>

namespace Platform::PCI {
    constexpr uint16_t configAddress = 0xCF8;
    constexpr uint16_t dataAddress = 0xCFC;

    uint32_t getAddress(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return (1U << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    }

    bool isAvailable() {
        return false; // TODO
    }

    template<>
    std::optional<uint8_t> readConfig<uint8_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        Arch::X86::PMIO::write<uint32_t>(configAddress, getAddress(bus, device, function, offset));

        return Arch::X86::PMIO::read<uint8_t>(dataAddress + (offset & 3));
    }
    
    template<>
    std::optional<uint16_t> readConfig<uint16_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        if (offset % 2) return std::nullopt;

        Arch::X86::PMIO::write<uint32_t>(configAddress, getAddress(bus, device, function, offset));

        return Arch::X86::PMIO::read<uint16_t>(dataAddress + (offset & 2));
    }

    template<>
    std::optional<uint32_t> readConfig<uint32_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        if (offset % 4) return std::nullopt;

        Arch::X86::PMIO::write<uint32_t>(configAddress, getAddress(bus, device, function, offset));

        return Arch::X86::PMIO::read<uint32_t>(dataAddress);
    }

    template<>
    bool writeConfig<uint8_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint8_t value) {
        Arch::X86::PMIO::write<uint32_t>(configAddress, getAddress(bus, device, function, offset));
        Arch::X86::PMIO::write<uint8_t>(dataAddress + (offset & 3), value);

        return true;
    }

    template<>
    bool writeConfig<uint16_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint16_t value) {
        if (offset % 2) return false;

        Arch::X86::PMIO::write<uint32_t>(configAddress, getAddress(bus, device, function, offset));
        Arch::X86::PMIO::write<uint16_t>(dataAddress + (offset & 2), value);

        return true;
    }

    template<>
    bool writeConfig<uint32_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value) {
        if (offset % 4) return false;

        Arch::X86::PMIO::write<uint32_t>(configAddress, getAddress(bus, device, function, offset));
        Arch::X86::PMIO::write<uint32_t>(dataAddress, value);
        
        return true;
    }
}