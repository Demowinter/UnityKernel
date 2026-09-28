#pragma once
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <drivers/fs/api.hpp>

namespace Driver::FS {
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
    };
}
