#include <drivers/driver.hpp>
#include <kernel/subsystems/block.hpp>

namespace Kernel::BlockSubsystem {
    struct BlockDeviceEntry {

    };

    void initialize() {}

    void finalize() {}

    void registerDriver(Driver::BaseDriver* instance) {
        if (instance->info().category != Driver::Category::BLK) return;
    }

    void unregisterDriver(Driver::BaseDriver* instance) {

    }

    STDLib::Vector<ResourceID> listBlockDevice() {

    }

    bool resizeBlockDevice(ResourceID id, std::size_t size) {

    }

    std::size_t readBlockDevice(ResourceID id,  void* dest, std::size_t size) {

    }

    std::size_t writeBlockDevice(ResourceID id, const void* src, std::size_t size) {

    }
    
    namespace Callbacks {
        
    }
}