#pragma once
#include <limits>
#include <cstddef>
#include <libstd/vector.hpp>
#include <drivers/driver.hpp>

namespace Kernel::BlockSubsystem {
    using ResourceID = std::size_t;

    constexpr ResourceID invalidID = std::numeric_limits<ResourceID>::max();

    void initialize();
    void finalize();

    void registerDriver(Driver::BaseDriver* instance);
    void unregisterDriver(Driver::BaseDriver* instance);

    STDLib::Vector<ResourceID> listBlockDevice();

    bool resizeBlockDevice(ResourceID id, std::size_t size);

    std::size_t readBlockDevice(ResourceID id,  void* dest, std::size_t size);
    std::size_t writeBlockDevice(ResourceID id, const void* src, std::size_t size);
}