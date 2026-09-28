#pragma once
#include <libstd/vector.hpp>
#include <drivers/api.hpp>
#include <drivers/blockdev/ramdisk.hpp>
#include <drivers/fs/fat32.hpp>

namespace Driver {
    const STDLib::Vector<BaseDriver*>& loadDriverList() {
        STDLib::Vector<BaseDriver*> drivers;
        drivers.push_back(new Blockdev::RamdiskDriver);
        drivers.push_back(new FS::FAT32Driver);

        return drivers;
    }
}