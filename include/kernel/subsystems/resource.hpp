#pragma once
#include <cstdint>
#include <libstd/vector.hpp>
#include <drivers/driver.hpp>

namespace Kernel::ResourceSubsystem {
    using ResourceID = uint32_t;

    constexpr ResourceID invalidID = -1U;

    enum ResourceType {
        DEVICE,
        SOMETHING_ELSE  // This line was written by the greatest technitian that ever lived...
    };

    struct ResourceInfo {
        ResourceID id;
        ResourceType type;

        uint32_t capabilities;

        Driver::BaseDriver* creator;
        Driver::BaseDriver* maintaner;
    };

    ResourceID registerResource(ResourceInfo info);
    void unregisterResource(ResourceID id);

    STDLib::Vector<ResourceID> getResourceList();

    ResourceType getType(ResourceID id);
    uint32_t getCapabilities(ResourceID id);

    Driver::BaseDriver* getCreator(ResourceID id);
    Driver::BaseDriver* getMaintaner(ResourceID id);

    void changeMaintaner(ResourceID id, Driver::BaseDriver* driver);
}