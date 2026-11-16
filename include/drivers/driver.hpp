#pragma once
#include <libstd/vector.hpp>

namespace Driver {
    enum class Category {
        DEV,    // Device driver category
        BLK,    // Blockdev driver category
        FS      // Filesystem driver category
    };

    enum class Type {
        ANY,
        RAMDISK,
        FAT32,
        EXT4,
        AHCI,
        UHC
    };

    struct Info {
        Category category;
        Type type;
    };

    class BaseDriver {
    public:
        BaseDriver(Category driCategory, Type driType) : driverInfo{driCategory, driType} {}
        virtual ~BaseDriver() = default;

        virtual bool initialize() = 0;
        virtual void finalize() = 0;

        Category category() const {
            return driverInfo.category;
        }

        Type type() const {
            return driverInfo.type;
        }

    protected:
        Info driverInfo;
    };
}