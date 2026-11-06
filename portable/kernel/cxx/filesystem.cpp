#include <kernel/filesystem.hpp>
#include <kernel/fat32.hpp>
#include <kernel/ext4.hpp>
#include <kernel/subsystems/driver.hpp>
#include <kernel/subsystems/resource.hpp>
#include <drivers/blockdev/ramdisk.hpp>
#include <libbase/memory.hpp>

namespace Kernel::Filesystem {
    namespace {
        enum class Backend {
            ext4Ram,
            ext4Sata,
            fat32Sata
        };

        constexpr uint32_t ramdiskSectors = 4096;
        Driver::Blockdev::RamdiskDriver* ramdisk = nullptr;
        Driver::FS::Ext4Driver ext4;
        ResourceSubsystem::ResourceID ramdiskResource = ResourceSubsystem::invalidID;
        Backend backend = Backend::ext4Ram;
        Type preferred = Type::ext4;
        bool initialized = false;

        Status fromExt4(Driver::FS::Status status) {
            switch (status) {
                case Driver::FS::Status::ok: return Status::ok;
                case Driver::FS::Status::notReady: return Status::notReady;
                case Driver::FS::Status::invalidName: return Status::invalidName;
                case Driver::FS::Status::notFound: return Status::notFound;
                case Driver::FS::Status::alreadyExists: return Status::alreadyExists;
                case Driver::FS::Status::notDirectory: return Status::notDirectory;
                case Driver::FS::Status::isDirectory: return Status::isDirectory;
                case Driver::FS::Status::directoryNotEmpty: return Status::directoryNotEmpty;
                case Driver::FS::Status::noSpace: return Status::noSpace;
                case Driver::FS::Status::ioError: return Status::ioError;
            }
            return Status::ioError;
        }

        Status fromFat32(FAT32::Status status) {
            switch (status) {
                case FAT32::Status::ok: return Status::ok;
                case FAT32::Status::notReady: return Status::notReady;
                case FAT32::Status::invalidName: return Status::invalidName;
                case FAT32::Status::notFound: return Status::notFound;
                case FAT32::Status::alreadyExists: return Status::alreadyExists;
                case FAT32::Status::notDirectory: return Status::notDirectory;
                case FAT32::Status::isDirectory: return Status::isDirectory;
                case FAT32::Status::directoryNotEmpty: return Status::directoryNotEmpty;
                case FAT32::Status::noSpace: return Status::noSpace;
                case FAT32::Status::ioError: return Status::ioError;
                case FAT32::Status::notMounted: return Status::notMounted;
                case FAT32::Status::invalidVolume: return Status::invalidVolume;
            }
            return Status::ioError;
        }

        template <typename SourceInfo>
        void copyInfo(const SourceInfo& source, EntryInfo& destination) {
            memcpy(destination.name, source.name, sizeof(destination.name));
            destination.directory = source.directory;
            destination.size = source.size;
            destination.cluster = source.cluster;
        }

        struct ListContext {
            ListCallback callback;
            void* context;
        };

        void ext4ListCallback(const Driver::FS::EntryInfo& entry, void* context) {
            auto& list = *static_cast<ListContext*>(context);
            EntryInfo info;
            copyInfo(entry, info);
            list.callback(info, list.context);
        }

        void fat32ListCallback(const FAT32::EntryInfo& entry, void* context) {
            auto& list = *static_cast<ListContext*>(context);
            EntryInfo info;
            copyInfo(entry, info);
            list.callback(info, list.context);
        }

        Status mountRamExt4() {
            if (ramdisk == nullptr || ramdiskResource == ResourceSubsystem::invalidID) {
                return Status::notReady;
            }
            ext4.useDriver(ramdisk);
            ext4.setResource(ramdiskResource);
            if (!ext4.initialize()) return Status::ioError;
            backend = Backend::ext4Ram;
            return Status::ok;
        }

        Status switchToRam() {
            if (backend == Backend::ext4Ram) return Status::ok;
            if (backend == Backend::ext4Sata) {
                ext4.finalize();
            } else {
                Status status = fromFat32(FAT32::unmountSata());
                if (status != Status::ok) return status;
            }
            return mountRamExt4();
        }
    }

    bool initialize() {
        FAT32::initialize();
        auto* blockDriver = DriverSubsystem::loadDriver(
            Driver::Category::BLK, Driver::Type::RAMDISK);
        ramdisk = static_cast<Driver::Blockdev::RamdiskDriver*>(blockDriver);
        if (ramdisk == nullptr) return false;

        const auto resources = ResourceSubsystem::getResourceList();
        for (ResourceSubsystem::ResourceID id : resources) {
            if (ResourceSubsystem::getCreator(id) == ramdisk) {
                ramdiskResource = id;
                break;
            }
        }
        if (ramdiskResource == ResourceSubsystem::invalidID) return false;

        ramdisk->resize(ramdiskResource, ramdiskSectors);
        ext4.useDriver(ramdisk);
        ext4.setResource(ramdiskResource);
        initialized = ext4.format() == Driver::FS::Status::ok;
        backend = Backend::ext4Ram;
        return initialized;
    }

    Status mountSata(Type type, Driver::Blockdev::BlockdevDriver& device) {
        if (!initialized) return Status::notReady;
        Status switchStatus = switchToRam();
        if (switchStatus != Status::ok) return switchStatus;

        if (type == Type::ext4) {
            ext4.useDriver(&device);
            ext4.setResource(0);
            if (ext4.initialize()) {
                backend = Backend::ext4Sata;
                return Status::ok;
            }
        } else {
            Status status = fromFat32(FAT32::mountSata(device));
            if (status == Status::ok) {
                backend = Backend::fat32Sata;
                return status;
            }
        }

        return mountRamExt4() == Status::ok ? Status::ioError : Status::notReady;
    }

    Status unmountSata() {
        if (backend == Backend::ext4Ram) return Status::notMounted;
        return switchToRam();
    }

    Status prepareDiskFormat() {
        if (!initialized) return Status::notReady;
        return switchToRam();
    }

    void setPreferredType(Type type) {
        preferred = type;
    }

    Type preferredType() {
        return preferred;
    }

    const char* statusText(Status status) {
        switch (status) {
            case Status::ok: return "success";
            case Status::notReady: return "filesystem is not ready";
            case Status::invalidName: return "invalid name";
            case Status::notFound: return "entry not found";
            case Status::alreadyExists: return "entry already exists";
            case Status::notDirectory: return "not a directory";
            case Status::isDirectory: return "is a directory";
            case Status::directoryNotEmpty: return "directory is not empty";
            case Status::noSpace: return "no space left on device";
            case Status::ioError: return "filesystem I/O error";
            case Status::notMounted: return "SATA filesystem is not mounted";
            case Status::invalidVolume: return "invalid filesystem volume";
        }
        return "unknown filesystem error";
    }

    std::string_view cwd() {
        return backend == Backend::fat32Sata ? FAT32::cwd() : ext4.cwd();
    }

    Status changeDirectory(std::string_view path) {
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::changeDirectory(path))
            : fromExt4(ext4.changeDirectory(path));
    }

    Status list(std::string_view path, ListCallback callback, void* context) {
        if (callback == nullptr) return Status::ioError;
        ListContext listContext{callback, context};
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::list(path, fat32ListCallback, &listContext))
            : fromExt4(ext4.listDirectory(path, ext4ListCallback, &listContext));
    }

    Status readFile(std::string_view path, ChunkCallback callback, void* context) {
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::readFile(path, callback, context))
            : fromExt4(ext4.readFile(path, callback, context));
    }

    Status writeFile(std::string_view path, std::string_view data, bool append) {
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::writeFile(path, data, append))
            : fromExt4(ext4.writeFile(path, data, append));
    }

    Status touch(std::string_view path) {
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::touch(path))
            : fromExt4(ext4.createFile(path));
    }

    Status makeDirectory(std::string_view path) {
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::makeDirectory(path))
            : fromExt4(ext4.createDirectory(path));
    }

    Status remove(std::string_view path) {
        return backend == Backend::fat32Sata
            ? fromFat32(FAT32::remove(path))
            : fromExt4(ext4.remove(path));
    }

    Status stat(std::string_view path, EntryInfo& info) {
        if (backend == Backend::fat32Sata) {
            FAT32::EntryInfo fatInfo;
            Status status = fromFat32(FAT32::stat(path, fatInfo));
            if (status == Status::ok) copyInfo(fatInfo, info);
            return status;
        }

        Driver::FS::EntryInfo ext4Info;
        Status status = fromExt4(ext4.stat(path, ext4Info));
        if (status == Status::ok) copyInfo(ext4Info, info);
        return status;
    }
}
