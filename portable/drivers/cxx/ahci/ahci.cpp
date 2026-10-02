#include <drivers/ahci/ahci.hpp>

namespace Driver::AHCI {
    namespace {
        constexpr uint32_t sataStatusDeviceDetectionMask = 0x0F;
        constexpr uint32_t sataStatusDevicePresent = 0x03;
        constexpr uint32_t sataStatusInterfacePowerMask = 0x0F00;
        constexpr uint32_t sataStatusInterfaceActive = 0x0100;

        constexpr uint32_t sataSignature = 0x00000101;
        constexpr uint32_t satapiSignature = 0xEB140101;
        constexpr uint32_t enclosureManagementSignature = 0xC33C0101;
        constexpr uint32_t portMultiplierSignature = 0x96690101;
    }

    bool Controller::initialize(volatile HBARegisters* hbaRegisters, size_t mappedSize) {
        registers = nullptr;

        if (hbaRegisters == nullptr ||
            mappedSize < hbaRegisterSize ||
            reinterpret_cast<uintptr_t>(hbaRegisters) % alignof(uint32_t) != 0) {
            return false;
        }

        registers = hbaRegisters;
        return true;
    }

    bool Controller::isInitialized() const {
        return registers != nullptr;
    }

    uint32_t Controller::implementedPortMask() const {
        return registers == nullptr ? 0 : registers->portsImplemented;
    }

    uint32_t Controller::implementedPortCount() const {
        uint32_t mask = implementedPortMask();
        uint32_t count = 0;

        while (mask != 0) {
            count += mask & 1;
            mask >>= 1;
        }
        return count;
    }

    PortType Controller::portType(uint8_t portNumber) const {
        volatile HBAPortRegisters* port = portRegisters(portNumber);
        if (port == nullptr) return PortType::NotImplemented;

        const uint32_t sataStatus = port->sataStatus;
        if ((sataStatus & sataStatusDeviceDetectionMask) != sataStatusDevicePresent ||
            (sataStatus & sataStatusInterfacePowerMask) != sataStatusInterfaceActive) {
            return PortType::NoDevice;
        }

        switch (port->signature) {
            case sataSignature: return PortType::SATA;
            case satapiSignature: return PortType::SATAPI;
            case enclosureManagementSignature: return PortType::EnclosureManagementBridge;
            case portMultiplierSignature: return PortType::PortMultiplier;
            default: return PortType::Unknown;
        }
    }

    volatile HBAPortRegisters* Controller::portRegisters(uint8_t portNumber) const {
        if (registers == nullptr ||
            portNumber >= portCount ||
            (registers->portsImplemented & (uint32_t{1} << portNumber)) == 0) {
            return nullptr;
        }
        return &registers->ports[portNumber];
    }
}
