#include <kernel/dformat.hpp>
#include <libbase/memory.hpp>

namespace Kernel::DFORMAT {
    namespace {
        constexpr uint32_t sectorSize = Driver::Blockdev::sectorSize;
        constexpr uint32_t partitionStart = 2048;
        constexpr uint32_t reservedSectors = 32;
        constexpr uint32_t fatCount = 2;
        constexpr uint32_t rootCluster = 2;
        constexpr uint32_t minFat32Clusters = 65525;
        constexpr uint32_t maxFat32Clusters = 0x0FFFFFF5;
        constexpr uint32_t maxSectorsPerCluster = 64;
        constexpr uint32_t zeroBufferSectors = 8;

        struct Geometry {
            uint32_t sectorsPerCluster;
            uint32_t fatSectors;
            uint32_t clusterCount;
        };

        void write16(uint8_t* buffer, uint32_t offset, uint16_t value) {
            buffer[offset] = static_cast<uint8_t>(value);
            buffer[offset + 1] = static_cast<uint8_t>(value >> 8);
        }

        void write32(uint8_t* buffer, uint32_t offset, uint32_t value) {
            buffer[offset] = static_cast<uint8_t>(value);
            buffer[offset + 1] = static_cast<uint8_t>(value >> 8);
            buffer[offset + 2] = static_cast<uint8_t>(value >> 16);
            buffer[offset + 3] = static_cast<uint8_t>(value >> 24);
        }

        Status chooseGeometry(uint32_t partitionSectors, Geometry& geometry) {
            bool hasEnoughClusters = false;

            for (uint32_t sectorsPerCluster = 1;
                 sectorsPerCluster <= maxSectorsPerCluster;
                 sectorsPerCluster *= 2) {
                uint32_t fatSectors = 1;
                uint32_t clusterCount = 0;
                bool converged = false;

                for (uint32_t iteration = 0; iteration < 32; iteration++) {
                    const uint64_t metadataSectors =
                        static_cast<uint64_t>(reservedSectors) +
                        static_cast<uint64_t>(fatCount) * fatSectors;
                    if (metadataSectors >= partitionSectors) break;

                    const uint32_t dataSectors =
                        partitionSectors - static_cast<uint32_t>(metadataSectors);
                    clusterCount = dataSectors / sectorsPerCluster;
                    const uint64_t fatBytes =
                        (static_cast<uint64_t>(clusterCount) + 2) * 4;
                    const uint32_t requiredFatSectors =
                        static_cast<uint32_t>((fatBytes + sectorSize - 1) / sectorSize);

                    if (requiredFatSectors == fatSectors) {
                        converged = true;
                        break;
                    }
                    fatSectors = requiredFatSectors;
                }

                if (!converged) continue;
                if (clusterCount < minFat32Clusters) continue;
                hasEnoughClusters = true;
                if (clusterCount > maxFat32Clusters) continue;

                geometry = {sectorsPerCluster, fatSectors, clusterCount};
                return Status::ok;
            }

            return hasEnoughClusters
                ? Status::unsupportedCapacity
                : Status::diskTooSmall;
        }

        bool clearSectors(Driver::Blockdev::BlockdevDriver& device,
                          uint32_t startLba, uint32_t sectorCount) {
            alignas(16) uint8_t zeros[zeroBufferSectors * sectorSize];
            memset(zeros, 0, sizeof(zeros));

            while (sectorCount != 0) {
                const uint32_t count = sectorCount < zeroBufferSectors
                    ? sectorCount
                    : zeroBufferSectors;
                if (!device.write(startLba, zeros, count)) return false;

                startLba += count;
                sectorCount -= count;
            }

            return true;
        }

        void makeBootSector(uint8_t* boot, uint32_t partitionSectors,
                            const Geometry& geometry) {
            memset(boot, 0, sectorSize);
            boot[0] = 0xEB;
            boot[1] = 0x58;
            boot[2] = 0x90;
            memcpy(boot + 3, "UNITYKRN", 8);

            write16(boot, 11, sectorSize);
            boot[13] = static_cast<uint8_t>(geometry.sectorsPerCluster);
            write16(boot, 14, reservedSectors);
            boot[16] = static_cast<uint8_t>(fatCount);
            write16(boot, 17, 0);
            write16(boot, 19, 0);
            boot[21] = 0xF8;
            write16(boot, 22, 0);
            write16(boot, 24, 63);
            write16(boot, 26, 255);
            write32(boot, 28, partitionStart);
            write32(boot, 32, partitionSectors);
            write32(boot, 36, geometry.fatSectors);
            write16(boot, 40, 0);
            write16(boot, 42, 0);
            write32(boot, 44, rootCluster);
            write16(boot, 48, 1);
            write16(boot, 50, 6);
            boot[64] = 0x80;
            boot[66] = 0x29;
            write32(boot, 67, 0x554B3332);
            memcpy(boot + 71, "UNITYKERN  ", 11);
            memcpy(boot + 82, "FAT32   ", 8);
            boot[510] = 0x55;
            boot[511] = 0xAA;
        }

        void makeFsInfo(uint8_t* fsInfo, uint32_t freeClusters) {
            memset(fsInfo, 0, sectorSize);
            write32(fsInfo, 0, 0x41615252);
            write32(fsInfo, 484, 0x61417272);
            write32(fsInfo, 488, freeClusters);
            write32(fsInfo, 492, rootCluster + 1);
            write32(fsInfo, 508, 0xAA550000);
        }

        bool writeFatHeader(Driver::Blockdev::BlockdevDriver& device,
                            uint32_t fatStart) {
            alignas(16) uint8_t fatSector[sectorSize] = {};
            write32(fatSector, 0, 0x0FFFFFF8);
            write32(fatSector, 4, 0x0FFFFFFF);
            write32(fatSector, rootCluster * 4, 0x0FFFFFFF);
            return device.write(fatStart, fatSector);
        }

        bool writeMbr(Driver::Blockdev::BlockdevDriver& device,
                      uint32_t partitionSectors) {
            alignas(16) uint8_t mbr[sectorSize] = {};
            uint8_t* partition = mbr + 446;
            partition[1] = 0xFE;
            partition[2] = 0xFF;
            partition[3] = 0xFF;
            partition[4] = 0x0C;
            partition[5] = 0xFE;
            partition[6] = 0xFF;
            partition[7] = 0xFF;
            write32(partition, 8, partitionStart);
            write32(partition, 12, partitionSectors);
            mbr[510] = 0x55;
            mbr[511] = 0xAA;
            return device.write(0, mbr);
        }
    }

    Status partitionAndFormatFat32(Driver::Blockdev::BlockdevDriver& device) {
        const uint32_t totalSectors = device.sectors();
        if (totalSectors <= partitionStart) return Status::invalidDevice;

        const uint32_t partitionSectors = totalSectors - partitionStart;
        Geometry geometry = {};
        const Status geometryStatus = chooseGeometry(partitionSectors, geometry);
        if (geometryStatus != Status::ok) return geometryStatus;

        if (!clearSectors(device, partitionStart, reservedSectors)) {
            return Status::ioError;
        }

        const uint32_t firstFat = partitionStart + reservedSectors;
        for (uint32_t index = 0; index < fatCount; index++) {
            const uint32_t fatStart = firstFat + index * geometry.fatSectors;
            if (!clearSectors(device, fatStart, geometry.fatSectors) ||
                !writeFatHeader(device, fatStart)) {
                return Status::ioError;
            }
        }

        alignas(16) uint8_t sector[sectorSize];
        makeBootSector(sector, partitionSectors, geometry);
        if (!device.write(partitionStart, sector) ||
            !device.write(partitionStart + 6, sector)) {
            return Status::ioError;
        }

        makeFsInfo(sector, geometry.clusterCount - 1);
        if (!device.write(partitionStart + 1, sector) ||
            !device.write(partitionStart + 7, sector)) {
            return Status::ioError;
        }

        const uint32_t rootStart = firstFat + fatCount * geometry.fatSectors;
        if (!clearSectors(device, rootStart,
                          geometry.sectorsPerCluster)) {
            return Status::ioError;
        }

        if (!writeMbr(device, partitionSectors)) return Status::ioError;
        return Status::ok;
    }

    const char* statusText(Status status) {
        switch (status) {
            case Status::ok: return "format completed";
            case Status::invalidDevice: return "SATA disk is unavailable or too small";
            case Status::diskTooSmall: return "disk is too small for a standard FAT32 volume";
            case Status::unsupportedCapacity: return "disk capacity is outside supported FAT32 limits";
            case Status::ioError: return "disk I/O failed while formatting";
        }

        return "unknown formatting error";
    }
}
