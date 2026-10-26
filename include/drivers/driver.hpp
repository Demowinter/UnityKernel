#pragma once
#include <libstd/vector.hpp>

namespace Driver {
    enum class Category {
        BUS,    // Bus driver category
        BLK,    // Blockdev driver category
        FS      // Filesystem driver category
    };

    enum class Type {
        ANY,
        PCI,
        USB,
        PS2,
        RAMDISK,
        FAT32,
        AHCI
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

        virtual bool canSatisfy(BaseDriver* driver) { return false; };
        virtual void useDriver(BaseDriver* driver) {};

        virtual STDLib::Vector<Info> dependencies() const { return {}; }

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