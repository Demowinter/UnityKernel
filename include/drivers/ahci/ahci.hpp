#pragma once
#include <cstddef>
#include <cstdint>
#include <drivers/blockdev/api.hpp>

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

    const char* lastInitializationError();

    class AHCIDriver final : public Blockdev::BlockdevDriver {
    public:
        AHCIDriver();
        ~AHCIDriver() override;

        bool initialize() override;
        void finalize() override;

        bool read(uint32_t lba, void* buffer, uint32_t count = 1) const override;
        bool write(uint32_t lba, const void* buffer, uint32_t count = 1) override;
        uint32_t sectors() const override;

        uint8_t portNumber() const;

    private:
        bool executeCommand(uint8_t command, uint64_t lba, uint16_t count,
                            bool write, void* buffer, size_t bufferSize) const;
        bool stopPort() const;
        void releaseDmaBuffers();

        volatile HBARegisters* registers = nullptr;
        volatile HBAPortRegisters* port = nullptr;
        uint8_t* commandList = nullptr;
        uint8_t* receivedFis = nullptr;
        uint8_t* commandTable = nullptr;
        void* commandListAllocation = nullptr;
        void* receivedFisAllocation = nullptr;
        void* commandTableAllocation = nullptr;
        uint64_t sectorCount = 0;
        uint8_t selectedPort = 0xFF;
    };
}
