#include <cstdint>
#include <libarch/x86/pmio.hpp>
#include <kernel/heap.hpp>
#include <drivers/ahci/ahci.hpp>

namespace Driver::AHCI {
    namespace {
        constexpr uint16_t pciConfigAddress = 0xCF8;
        constexpr uint16_t pciConfigData = 0xCFC;
        constexpr uint32_t pciAddressEnable = 0x80000000;
        constexpr uint32_t pciBar5Offset = 0x24;
        constexpr uint16_t pciCommandOffset = 0x04;
        constexpr uint16_t pciCommandMemorySpace = 1U << 1;
        constexpr uint16_t pciCommandBusMaster = 1U << 2;
        constexpr uint32_t ahciClassCode = 0x010601;
        constexpr uint32_t ahciEnable = 1U << 31;
        constexpr uint32_t hbaReset = 1U;
        constexpr uint32_t hbaInterruptEnable = 1U << 1;
        constexpr uint32_t hbaBiosOwned = 1U;
        constexpr uint32_t hbaOsOwned = 1U << 1;
        constexpr uint32_t hbaBiosBusy = 1U << 4;
        constexpr uint32_t portStart = 1U;
        constexpr uint32_t portFisReceiveEnable = 1U << 4;
        constexpr uint32_t portCommandListRunning = 1U << 15;
        constexpr uint32_t portFisReceiveRunning = 1U << 14;
        constexpr uint32_t portTaskFileError = 1U;
        constexpr uint32_t portTaskFileBusy = 1U << 7;
        constexpr uint32_t portTaskFileDataRequest = 1U << 3;
        constexpr uint32_t portTaskFileBusyMask = portTaskFileBusy | portTaskFileDataRequest;
        constexpr uint32_t portInterruptTaskFileError = 1U << 30;
        constexpr uint32_t sataStatusDeviceDetectionMask = 0x0F;
        constexpr uint32_t sataStatusDevicePresent = 0x03;
        constexpr uint32_t sataStatusInterfacePowerMask = 0x0F00;
        constexpr uint32_t sataStatusInterfaceActive = 0x0100;
        constexpr uint32_t sataSignature = 0x00000101;
        constexpr uint32_t hbaBohCapability = 1U;
        constexpr uint8_t fisRegisterHostToDevice = 0x27;
        constexpr uint8_t fisCommandFlag = 1U << 7;
        constexpr uint8_t ataIdentifyDevice = 0xEC;
        constexpr uint8_t ataReadDmaExt = 0x25;
        constexpr uint8_t ataWriteDmaExt = 0x35;
        constexpr uint8_t identifyLba48Word = 83;
        constexpr uint16_t identifyLba48Bit = 1U << 10;
        constexpr uint16_t identifyCommandSetValidityMask = 3U << 14;
        constexpr uint16_t identifyCommandSetValid = 1U << 14;
        constexpr uint16_t identifyDmaSupportWord = 49;
        constexpr uint16_t identifyDmaSupportBit = 1U << 8;
        constexpr uint8_t identifySectorSizeWord = 106;
        constexpr uint16_t identifySectorSizeValid = 1U << 14;
        constexpr uint16_t identifySectorSizeValidityMask = 3U << 14;
        constexpr uint16_t identifyLongLogicalSector = 1U << 12;
        constexpr uint16_t identifyLogicalSectorWordsLow = 117;
        constexpr size_t identifyBytes = 512;
        constexpr size_t maxPrdtBytes = 4 * 1024 * 1024;
        constexpr uint32_t maxSectorsPerCommand = maxPrdtBytes / Blockdev::sectorSize;
        constexpr uint64_t pollLimit = 10000000;
        constexpr size_t commandListBytes = 1024;
        constexpr size_t receivedFisBytes = 256;
        constexpr size_t commandTableBytes = 256;

        struct __attribute__((packed)) CommandHeader {
            uint16_t flags;
            uint16_t prdtLength;
            uint32_t transferredBytes;
            uint32_t commandTableBase;
            uint32_t commandTableBaseUpper;
            uint32_t reserved[4];
        };

        struct __attribute__((packed)) PrdtEntry {
            uint32_t dataBase;
            uint32_t dataBaseUpper;
            uint32_t reserved;
            uint32_t byteCountAndInterrupt;
        };

        struct __attribute__((packed)) CommandTable {
            uint8_t commandFis[64];
            uint8_t atapiCommand[16];
            uint8_t reserved[48];
            PrdtEntry prdt;
        };

        static_assert(sizeof(CommandHeader) == 32);
        static_assert(sizeof(PrdtEntry) == 16);
        static_assert(offsetof(CommandTable, prdt) == 128);
        static_assert(sizeof(HBARegisters) == hbaRegisterSize);

        struct PciDevice {
            uint8_t bus;
            uint8_t device;
            uint8_t function;
        };

        uint32_t pciRead32(const PciDevice& device, uint8_t offset) {
            const uint32_t address = pciAddressEnable |
                (static_cast<uint32_t>(device.bus) << 16) |
                (static_cast<uint32_t>(device.device) << 11) |
                (static_cast<uint32_t>(device.function) << 8) |
                (offset & 0xFC);
            Arch::X86::PMIO::write<uint32_t>(pciConfigAddress, address);
            return Arch::X86::PMIO::read<uint32_t>(pciConfigData);
        }

        uint16_t pciRead16(const PciDevice& device, uint8_t offset) {
            const uint32_t value = pciRead32(device, offset);
            return static_cast<uint16_t>(value >> ((offset & 2) * 8));
        }

        void pciWrite16(const PciDevice& device, uint8_t offset, uint16_t value) {
            const uint32_t address = pciAddressEnable |
                (static_cast<uint32_t>(device.bus) << 16) |
                (static_cast<uint32_t>(device.device) << 11) |
                (static_cast<uint32_t>(device.function) << 8) |
                (offset & 0xFC);
            Arch::X86::PMIO::write<uint32_t>(pciConfigAddress, address);
            const uint32_t shift = (offset & 2) * 8;
            const uint32_t mask = 0xFFFFU << shift;
            const uint32_t oldValue = Arch::X86::PMIO::read<uint32_t>(pciConfigData);
            uint32_t preservedValue = oldValue & ~mask;
            if ((offset & 0xFC) == pciCommandOffset) preservedValue &= 0x0000FFFF;
            const uint32_t newValue = preservedValue |
                (static_cast<uint32_t>(value) << shift);
            Arch::X86::PMIO::write<uint32_t>(pciConfigData, newValue);
        }

        bool findAhciPciDevice(PciDevice& result) {
            for (uint32_t bus = 0; bus < 256; ++bus) {
                for (uint32_t device = 0; device < 32; ++device) {
                    PciDevice candidate{
                        static_cast<uint8_t>(bus),
                        static_cast<uint8_t>(device),
                        0
                    };
                    const uint32_t id = pciRead32(candidate, 0);
                    if (static_cast<uint16_t>(id) == 0xFFFF) continue;

                    const uint32_t header = pciRead32(candidate, 0x0C);
                    const uint32_t functionCount = (header & (1U << 23)) ? 8 : 1;
                    for (uint32_t function = 0; function < functionCount; ++function) {
                        candidate.function = static_cast<uint8_t>(function);
                        if (static_cast<uint16_t>(pciRead32(candidate, 0)) == 0xFFFF) continue;

                        const uint32_t classInfo = (pciRead32(candidate, 0x08) >> 8) & 0xFFFFFF;
                        if (classInfo == ahciClassCode) {
                            result = candidate;
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        bool getAhciBar(const PciDevice& device, uint32_t& base, uint32_t& size) {
            const uint32_t bar = pciRead32(device, pciBar5Offset);
            if (bar == 0 || bar == 0xFFFFFFFF || (bar & 1) != 0 || (bar & 6) != 0) return false;

            const uint16_t originalCommand = pciRead16(device, pciCommandOffset);
            pciWrite16(device, pciCommandOffset, originalCommand & ~pciCommandMemorySpace);
            const uint32_t originalBar = pciRead32(device, pciBar5Offset);
            {
                const uint32_t address = pciAddressEnable |
                    (static_cast<uint32_t>(device.bus) << 16) |
                    (static_cast<uint32_t>(device.device) << 11) |
                    (static_cast<uint32_t>(device.function) << 8) |
                    (pciBar5Offset & 0xFC);
                Arch::X86::PMIO::write<uint32_t>(pciConfigAddress, address);
                Arch::X86::PMIO::write<uint32_t>(pciConfigData, 0xFFFFFFFF);
            }
            const uint32_t mask = pciRead32(device, pciBar5Offset);
            const uint32_t restoreAddress = pciAddressEnable |
                (static_cast<uint32_t>(device.bus) << 16) |
                (static_cast<uint32_t>(device.device) << 11) |
                (static_cast<uint32_t>(device.function) << 8) |
                (pciBar5Offset & 0xFC);
            Arch::X86::PMIO::write<uint32_t>(pciConfigAddress, restoreAddress);
            Arch::X86::PMIO::write<uint32_t>(pciConfigData, originalBar);
            pciWrite16(device, pciCommandOffset, originalCommand);

            const uint32_t barMask = mask & 0xFFFFFFF0;
            const uint32_t barSize = ~barMask + 1;
            if (barMask == 0 || barSize < hbaRegisterSize ||
                (barSize & (barSize - 1)) != 0) {
                return false;
            }

            base = bar & 0xFFFFFFF0;
            size = barSize;
            return base != 0;
        }

        void* allocateAligned(size_t size, size_t alignment, void*& allocation) {
            if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0 ||
                size > static_cast<size_t>(-1) - (alignment - 1)) {
                return nullptr;
            }
            allocation = Kernel::Heap::allocate(size + alignment - 1);
            if (allocation == nullptr) return nullptr;

            const uintptr_t rawAddress = reinterpret_cast<uintptr_t>(allocation);
            const uintptr_t alignedAddress = (rawAddress + alignment - 1) & ~(alignment - 1);
            if (alignedAddress > 0xFFFFFFFFU ||
                size - 1 > 0xFFFFFFFFU - alignedAddress) {
                Kernel::Heap::deallocate(allocation);
                allocation = nullptr;
                return nullptr;
            }
            return reinterpret_cast<void*>(alignedAddress);
        }

        bool waitForClear(volatile uint32_t& value, uint32_t mask) {
            for (uint64_t i = 0; i < pollLimit; ++i) {
                if ((value & mask) == 0) return true;
                asm volatile("pause");
            }
            return false;
        }

        bool stopPortEngine(volatile HBAPortRegisters* port) {
            if (port == nullptr) return true;
            port->command &= ~portStart;
            const bool commandStopped = waitForClear(port->command, portCommandListRunning);
            if (!commandStopped) return false;

            port->command &= ~portFisReceiveEnable;
            const bool fisStopped = waitForClear(port->command, portFisReceiveRunning);
            return fisStopped;
        }

        PortType classifyPort(volatile HBAPortRegisters* port) {
            const uint32_t sataStatus = port->sataStatus;
            if ((sataStatus & sataStatusDeviceDetectionMask) != sataStatusDevicePresent ||
                (sataStatus & sataStatusInterfacePowerMask) != sataStatusInterfaceActive) {
                return PortType::NoDevice;
            }
            return port->signature == sataSignature ? PortType::SATA : PortType::Unknown;
        }

        uint64_t physicalAddress(const void* pointer) {
            return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pointer));
        }

        void zeroMemory(void* memory, size_t size) {
            auto* bytes = static_cast<uint8_t*>(memory);
            for (size_t i = 0; i < size; ++i) bytes[i] = 0;
        }
    }

    AHCIDriver::AHCIDriver() : Blockdev::BlockdevDriver{Type::AHCI} {}

    AHCIDriver::~AHCIDriver() {
        finalize();
    }

    bool AHCIDriver::initialize() {
        finalize();
        if (registers != nullptr || commandListAllocation != nullptr ||
            receivedFisAllocation != nullptr || commandTableAllocation != nullptr) {
            return false;
        }

        PciDevice pciDevice{};
        if (!findAhciPciDevice(pciDevice)) return false;

        uint32_t abar = 0;
        uint32_t abarSize = 0;
        if (!getAhciBar(pciDevice, abar, abarSize)) return false;

        uint16_t pciCommand = pciRead16(pciDevice, pciCommandOffset);
        pciCommand |= pciCommandMemorySpace | pciCommandBusMaster;
        pciWrite16(pciDevice, pciCommandOffset, pciCommand);

        registers = reinterpret_cast<volatile HBARegisters*>(static_cast<uintptr_t>(abar));
        if (registers == nullptr || abarSize < hbaRegisterSize) {
            registers = nullptr;
            return false;
        }

        if ((registers->capabilities2 & hbaBohCapability) != 0) {
            registers->biosHandoffControl |= hbaOsOwned;
            if (!waitForClear(registers->biosHandoffControl, hbaBiosOwned | hbaBiosBusy)) {
                registers = nullptr;
                return false;
            }
        }

        registers->globalHostControl |= ahciEnable;
        registers->globalHostControl |= hbaReset;
        if (!waitForClear(registers->globalHostControl, hbaReset)) {
            registers = nullptr;
            return false;
        }
        registers->globalHostControl =
            (registers->globalHostControl | ahciEnable) & ~hbaInterruptEnable;

        const uint32_t implemented = registers->portsImplemented;
        for (uint8_t portNumber = 0; portNumber < portCount; ++portNumber) {
            if ((implemented & (uint32_t{1} << portNumber)) == 0) continue;

            volatile HBAPortRegisters* candidate = &registers->ports[portNumber];
            if (classifyPort(candidate) != PortType::SATA) continue;
            if (!stopPortEngine(candidate)) continue;

            port = candidate;
            selectedPort = portNumber;

            commandList = static_cast<uint8_t*>(
                allocateAligned(commandListBytes, 1024, commandListAllocation));
            receivedFis = static_cast<uint8_t*>(
                allocateAligned(receivedFisBytes, 256, receivedFisAllocation));
            commandTable = static_cast<uint8_t*>(
                allocateAligned(commandTableBytes, 128, commandTableAllocation));

            if (commandList == nullptr || receivedFis == nullptr || commandTable == nullptr) {
                finalize();
                if (port != nullptr) return false;
                continue;
            }

            zeroMemory(commandList, commandListBytes);
            zeroMemory(receivedFis, receivedFisBytes);
            zeroMemory(commandTable, commandTableBytes);

            port->commandListBase = static_cast<uint32_t>(physicalAddress(commandList));
            port->commandListBaseUpper = 0;
            port->fisBase = static_cast<uint32_t>(physicalAddress(receivedFis));
            port->fisBaseUpper = 0;
            port->interruptEnable = 0;
            port->interruptStatus = 0xFFFFFFFF;
            port->sataError = 0xFFFFFFFF;

            port->command |= portFisReceiveEnable;
            port->command |= portStart;

            void* identifyAllocation = nullptr;
            auto* identifyData = static_cast<uint16_t*>(
                allocateAligned(identifyBytes, 2, identifyAllocation));
            if (identifyData == nullptr) {
                finalize();
                if (port != nullptr) return false;
                continue;
            }

            zeroMemory(identifyData, identifyBytes);
            const bool identified = executeCommand(
                ataIdentifyDevice, 0, 1, false, identifyData, identifyBytes);

            bool has512ByteLogicalSectors = true;
            if (identified &&
                (identifyData[identifySectorSizeWord] & identifySectorSizeValidityMask) ==
                    identifySectorSizeValid &&
                (identifyData[identifySectorSizeWord] & identifyLongLogicalSector) != 0) {
                const uint32_t logicalSectorWords =
                    static_cast<uint32_t>(identifyData[identifyLogicalSectorWordsLow]) |
                    (static_cast<uint32_t>(identifyData[identifyLogicalSectorWordsLow + 1]) << 16);
                has512ByteLogicalSectors = logicalSectorWords == 256;
            }

            if (identified &&
                has512ByteLogicalSectors &&
                (identifyData[identifyDmaSupportWord] & identifyDmaSupportBit) != 0 &&
                (identifyData[identifyLba48Word] & identifyCommandSetValidityMask) ==
                    identifyCommandSetValid &&
                (identifyData[identifyLba48Word] & identifyLba48Bit) != 0) {
                sectorCount = static_cast<uint64_t>(identifyData[100]) |
                    (static_cast<uint64_t>(identifyData[101]) << 16) |
                    (static_cast<uint64_t>(identifyData[102]) << 32) |
                    (static_cast<uint64_t>(identifyData[103]) << 48);
            } else {
                sectorCount = 0;
            }

            Kernel::Heap::deallocate(identifyAllocation);
            if (sectorCount != 0) return true;

            finalize();
            if (port != nullptr) return false;
        }

        return false;
    }

    void AHCIDriver::finalize() {
        const bool portStopped = stopPortEngine(port);
        if (!portStopped) return;

        if (port != nullptr) {
            port->commandListBase = 0;
            port->commandListBaseUpper = 0;
            port->fisBase = 0;
            port->fisBaseUpper = 0;
        }
        releaseDmaBuffers();
        port = nullptr;
        registers = nullptr;
        sectorCount = 0;
        selectedPort = 0xFF;
    }

    void AHCIDriver::releaseDmaBuffers() {
        if (commandListAllocation != nullptr) Kernel::Heap::deallocate(commandListAllocation);
        if (receivedFisAllocation != nullptr) Kernel::Heap::deallocate(receivedFisAllocation);
        if (commandTableAllocation != nullptr) Kernel::Heap::deallocate(commandTableAllocation);
        commandListAllocation = nullptr;
        receivedFisAllocation = nullptr;
        commandTableAllocation = nullptr;
        commandList = nullptr;
        receivedFis = nullptr;
        commandTable = nullptr;
    }

    bool AHCIDriver::executeCommand(uint8_t command, uint64_t lba, uint16_t count,
                                   bool isWrite, void* buffer, size_t bufferSize) const {
        if (port == nullptr || commandList == nullptr || commandTable == nullptr ||
            buffer == nullptr || bufferSize == 0 || bufferSize > maxPrdtBytes) {
            return false;
        }
        const uint64_t bufferAddress = physicalAddress(buffer);
        if (bufferAddress > 0xFFFFFFFFU ||
            bufferSize - 1 > 0xFFFFFFFFU - bufferAddress) {
            return false;
        }

        auto* headers = reinterpret_cast<CommandHeader*>(commandList);
        CommandHeader& header = headers[0];
        auto* table = reinterpret_cast<CommandTable*>(commandTable);

        if (!waitForClear(port->taskFileData, portTaskFileBusyMask) ||
            !waitForClear(port->commandIssue, 1U)) {
            return false;
        }

        zeroMemory(table, sizeof(CommandTable));
        header.flags = 5 | (isWrite ? (1U << 6) : 0);
        header.prdtLength = 1;
        header.transferredBytes = 0;
        header.commandTableBase = static_cast<uint32_t>(physicalAddress(commandTable));
        header.commandTableBaseUpper = 0;

        table->commandFis[0] = fisRegisterHostToDevice;
        table->commandFis[1] = fisCommandFlag;
        table->commandFis[2] = command;

        if (command != ataIdentifyDevice) {
            table->commandFis[7] = 1U << 6;
            table->commandFis[4] = static_cast<uint8_t>(lba);
            table->commandFis[5] = static_cast<uint8_t>(lba >> 8);
            table->commandFis[6] = static_cast<uint8_t>(lba >> 16);
            table->commandFis[8] = static_cast<uint8_t>(lba >> 24);
            table->commandFis[9] = static_cast<uint8_t>(lba >> 32);
            table->commandFis[10] = static_cast<uint8_t>(lba >> 40);
            table->commandFis[12] = static_cast<uint8_t>(count);
            table->commandFis[13] = static_cast<uint8_t>(count >> 8);
        }

        table->prdt.dataBase = static_cast<uint32_t>(bufferAddress);
        table->prdt.dataBaseUpper = 0;
        table->prdt.byteCountAndInterrupt = static_cast<uint32_t>(bufferSize - 1) | (1U << 31);

        port->interruptStatus = 0xFFFFFFFF;
        port->sataError = 0xFFFFFFFF;
        asm volatile("mfence" ::: "memory");
        port->commandIssue |= 1U;

        for (uint64_t i = 0; i < pollLimit; ++i) {
            if ((port->interruptStatus & portInterruptTaskFileError) != 0 ||
                (port->taskFileData & portTaskFileError) != 0) {
                return false;
            }
            if ((port->commandIssue & 1U) == 0) {
                return waitForClear(port->taskFileData, portTaskFileBusyMask) &&
                    header.transferredBytes == bufferSize;
            }
            asm volatile("pause");
        }
        return false;
    }

    bool AHCIDriver::read(uint32_t lba, void* buffer, uint32_t count) const {
        if (buffer == nullptr || count == 0 || count > maxSectorsPerCommand ||
            lba > sectors() || count > sectors() - lba) {
            return false;
        }
        return executeCommand(ataReadDmaExt, lba, static_cast<uint16_t>(count),
                              false, buffer, static_cast<size_t>(count) * Blockdev::sectorSize);
    }

    bool AHCIDriver::write(uint32_t lba, const void* buffer, uint32_t count) {
        if (buffer == nullptr || count == 0 || count > maxSectorsPerCommand ||
            lba > sectors() || count > sectors() - lba) {
            return false;
        }
        return executeCommand(ataWriteDmaExt, lba, static_cast<uint16_t>(count),
                              true, const_cast<void*>(buffer),
                              static_cast<size_t>(count) * Blockdev::sectorSize);
    }

    uint32_t AHCIDriver::sectors() const {
        return sectorCount > 0xFFFFFFFFULL
            ? 0xFFFFFFFFU
            : static_cast<uint32_t>(sectorCount);
    }

    uint8_t AHCIDriver::portNumber() const {
        return selectedPort;
    }
}
