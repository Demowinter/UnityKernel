#pragma once
#include <string_view>
#include <libstd/vector.hpp>
#include <drivers/api.hpp>
#include <drivers/blockdev/api.hpp>

namespace Driver::FS {
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

    class FSDriver : public BaseDriver {
    public:
        FSDriver(Type driType) : BaseDriver{Category::FS, driType} {}
        virtual ~FSDriver() = default;

        virtual bool canSatisfy(BaseDriver* driver) override { return true; };
        virtual void useDriver(BaseDriver* driver) override { this->driver = static_cast<Blockdev::BlockdevDriver*>(driver); };

        virtual const STDLib::Vector<Info>& dependencies() const override { return {{Category::BLK}}; }

        const char* statusText(Status status);

        virtual std::string_view cwd() = 0;

        virtual Status changeDirectory(std::string_view path) = 0;
        virtual Status listDirectory(std::string_view path, ListCallback callback, void* context) = 0;

        virtual Status readFile(std::string_view path, ChunkCallback callback, void* context) = 0;
        virtual Status writeFile(std::string_view path, std::string_view data, bool append) = 0;

        virtual Status createFile(std::string_view path) = 0;
        virtual Status createDirectory(std::string_view path) = 0;

        virtual Status remove(std::string_view path) = 0;

        virtual Status stat(std::string_view path, EntryInfo& info) = 0;

    private:
        Blockdev::BlockdevDriver* driver;
    };
}