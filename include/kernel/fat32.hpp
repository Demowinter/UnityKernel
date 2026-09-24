#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Kernel::FAT32 {
    enum class Status {
        ok,
        notReady,
        invalidName,
        notFound,
        alreadyExists,
        notDirectory,
        isDirectory,
        directoryNotEmpty,
        noSpace,
        ioError
    };

    struct EntryInfo {
        char name[13];
        bool directory;
        uint32_t size;
        uint32_t cluster;
    };

    using ListCallback = void (*)(const EntryInfo& entry, void* context);
    using ChunkCallback = void (*)(std::string_view chunk, void* context);

    void initialize();
    bool ready();

    const char* statusText(Status status);
    std::string_view cwd();

    Status changeDirectory(std::string_view path);
    Status list(std::string_view path, ListCallback callback, void* context);
    Status readFile(std::string_view path, ChunkCallback callback, void* context);
    Status writeFile(std::string_view path, std::string_view data, bool append);
    Status touch(std::string_view path);
    Status makeDirectory(std::string_view path);
    Status remove(std::string_view path);
    Status stat(std::string_view path, EntryInfo& info);
}
