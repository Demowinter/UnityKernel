#pragma once
#include <cstddef>
#include <cstdint>

namespace Driver::AHCI {
    constexpr size_t portCount = 32;
    constexpr size_t portRegisterOffset = 0x100;
    constexpr size_t portRegisterSize = 0x80;
    constexpr size_t hbaRegisterSize = portRegisterOffset + portCount * portRegisterSize;

    struct HBAPortRegisters {
        volatile uint32_t commandListBase;
        volatile uint32_t commandListBaseUpper;
        volatile uint32_t fisBase;
        volatile uint32_t fisBaseUpper;
        volatile uint32_t interruptStatus;
        volatile uint32_t interruptEnable;
        volatile uint32_t command;
        uint32_t reserved0;
        volatile uint32_t taskFileData;
        volatile uint32_t signature;
        volatile uint32_t sataStatus;
        volatile uint32_t sataControl;
        volatile uint32_t sataError;
        volatile uint32_t sataActive;
        volatile uint32_t commandIssue;
        volatile uint32_t sataNotification;
        volatile uint32_t fisBasedSwitchControl;
        volatile uint32_t deviceSleep;
        uint32_t reserved1[10];
        uint32_t vendor[4];
    };

    struct HBARegisters {
        volatile uint32_t capabilities;
        volatile uint32_t globalHostControl;
        volatile uint32_t interruptStatus;
        volatile uint32_t portsImplemented;
        volatile uint32_t version;
        volatile uint32_t commandCompletionCoalescingControl;
        volatile uint32_t commandCompletionCoalescingPorts;
        volatile uint32_t enclosureManagementLocation;
        volatile uint32_t enclosureManagementControl;
        volatile uint32_t capabilities2;
        volatile uint32_t biosHandoffControl;
        uint32_t reserved[29];
        uint32_t vendor[24];
        HBAPortRegisters ports[portCount];
    };

    static_assert(sizeof(HBAPortRegisters) == portRegisterSize);
    static_assert(offsetof(HBARegisters, ports) == portRegisterOffset);
    static_assert(sizeof(HBARegisters) == hbaRegisterSize);

    enum class PortType {
        NotImplemented,
        NoDevice,
        SATA,
        SATAPI,
        EnclosureManagementBridge,
        PortMultiplier,
        Unknown
    };

    class Controller {
    public:
        bool initialize(volatile HBARegisters* registers, size_t mappedSize);
        bool isInitialized() const;
        uint32_t implementedPortMask() const;
        uint32_t implementedPortCount() const;
        PortType portType(uint8_t portNumber) const;
        volatile HBAPortRegisters* portRegisters(uint8_t portNumber) const;

    private:
        volatile HBARegisters* registers = nullptr;
    };
}
