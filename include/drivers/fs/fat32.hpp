#pragma once
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <libstd/string.hpp>
#include <drivers/fs/api.hpp>

namespace Driver::FS {
    namespace FAT32 {
        constexpr uint8_t sectorsPerCluster = 1;
        constexpr uint32_t reservedSectors = 32;
        constexpr uint32_t fatSectors = 2;
        constexpr uint32_t clusterCount = 128;
        constexpr uint32_t rootCluster = 2;
        constexpr uint32_t totalSectors = reservedSectors + fatSectors + clusterCount * sectorsPerCluster;
        constexpr uint32_t clusterBytes = Driver::Blockdev::sectorSize * sectorsPerCluster;
        constexpr uint32_t diskBytes = totalSectors * Driver::Blockdev::sectorSize;

        constexpr uint32_t endOfChain = 0x0FFFFFFF;
        constexpr uint32_t endOfChainMin = 0x0FFFFFF8;
        constexpr uint8_t attrReadOnly = 0x01;
        constexpr uint8_t attrHidden = 0x02;
        constexpr uint8_t attrSystem = 0x04;
        constexpr uint8_t attrVolumeId = 0x08;
        constexpr uint8_t attrDirectory = 0x10;
        constexpr uint8_t attrArchive = 0x20;
        constexpr uint8_t attrLongName = attrReadOnly | attrHidden | attrSystem | attrVolumeId;
    }

    class FAT32Driver : public FSDriver {
    public:
        FAT32Driver() : FSDriver{Type::FAT32} {};

        bool initialize() override;
        void finalize() override;

        std::string_view cwd() override;

        Status changeDirectory(std::string_view path) override;
        Status listDirectory(std::string_view path, ListCallback callback, void* context) override;

        Status readFile(std::string_view path, ChunkCallback callback, void* context) override;
        Status writeFile(std::string_view path, std::string_view data, bool append) override;

        Status createFile(std::string_view path) override;
        Status createDirectory(std::string_view path) override;

        Status remove(std::string_view path) override;

        Status stat(std::string_view path, EntryInfo& info) override;

        Status format() override;

    private:
        bool mounted = false;

        uint32_t cwdCluster = FAT32::rootCluster;

        STDLib::String cwdPath;
    };
}
