#pragma once
#include <initializer_list>
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
        FAT32
    };

    struct Info {
        Category category;
        Type type;
    };

    class BaseDriver {
    public:
        BaseDriver(Category driCategory, Type driType, std::initializer_list<Info> deps = {});
        virtual ~BaseDriver() = default;

        virtual bool initialize() = 0;
        virtual void finalize() = 0;

        virtual bool canSatisfy(BaseDriver* driver) = 0;
        virtual void useDriver(BaseDriver* driver) = 0;

        Category category() const;
        Type type() const;

        const STDLib::Vector<Info>& dependencies() const;

    private:
        Info driverInfo;

        STDLib::Vector<Info> driverDependencies;
    };
}