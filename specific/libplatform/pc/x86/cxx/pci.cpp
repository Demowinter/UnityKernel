#include <optional>
#include <cstdint>
#include <libarch/x86/pmio.hpp>
#include <libplatform/pc/x86/pci/cm1.hpp>
#include <libplatform/pc/x86/pci/ecam.hpp>
#include <libplatform/pci.hpp>

namespace Platform::PCI {
    static bool CM1Fallback = false;

    void initialize() {
        CM1Fallback = !ECAM::isAvailable();
    }

    bool isAvailable() {
        return CM1::isAvailable() || ECAM::isAvailable();
    }

    template<>
    std::optional<uint8_t> readConfig<uint8_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return (CM1Fallback) ? CM1::readConfig<uint8_t>(bus, device, function, offset) : ECAM::readConfig<uint8_t>(bus, device, function, offset);
    }
    
    template<>
    std::optional<uint16_t> readConfig<uint16_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return (CM1Fallback) ? CM1::readConfig<uint16_t>(bus, device, function, offset) : ECAM::readConfig<uint16_t>(bus, device, function, offset);
    }

    template<>
    std::optional<uint32_t> readConfig<uint32_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
        return (CM1Fallback) ? CM1::readConfig<uint32_t>(bus, device, function, offset) : ECAM::readConfig<uint32_t>(bus, device, function, offset);
    }

    template<>
    bool writeConfig<uint8_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint8_t value) {
        return (CM1Fallback) ? CM1::writeConfig<uint8_t>(bus, device, function, offset, value) : ECAM::writeConfig<uint8_t>(bus, device, function, offset, value);
    }

    template<>
    bool writeConfig<uint16_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint16_t value) {
        return (CM1Fallback) ? CM1::writeConfig<uint16_t>(bus, device, function, offset, value) : ECAM::writeConfig<uint16_t>(bus, device, function, offset, value);
    }

    template<>
    bool writeConfig<uint32_t>(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value) {
        return (CM1Fallback) ? CM1::writeConfig<uint32_t>(bus, device, function, offset, value) : ECAM::writeConfig<uint32_t>(bus, device, function, offset, value);
    }
}