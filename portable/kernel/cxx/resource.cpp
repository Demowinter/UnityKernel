#include <limits>
#include <libstd/vector.hpp>
#include <libstd/map.hpp>
#include <drivers/driver.hpp>
#include <kernel/system.hpp>
#include <kernel/subsystems/resource.hpp>

namespace Kernel::ResourceSubsystem {
    static STDLib::Map<ResourceID, ResourceInfo> resources;
    static STDLib::Vector<ResourceID> freeIds;

    ResourceID getFreeId() {
        static ResourceID nextId = 0;

        ResourceID id = invalidID;

        if (freeIds.size()) {
            auto it = freeIds.begin();

            id = *it;

            freeIds.erase(it);
        }

        else if (nextId < std::numeric_limits<ResourceID>::max()) id = nextId++;

        return id;
    }

    //---------------------API---------------------

    ResourceID registerResource(ResourceInfo info) {
        info.id = getFreeId();

        if (info.id == invalidID) System::panic("Kernel::ResourceSubsystem::registerResource()", "Too many resources");

        resources.insert(info.id, info);

        return info.id;
    }

    void unregisterResource(ResourceID id) {
        if (!resources.find(id)) System::panic("Kernel::ResourceSubsystem::unregisterResource()", "Invalid ID");

        resources.erase(id);
        freeIds.push_back(id);
    }

    STDLib::Vector<ResourceID> getResourceList() {
        STDLib::Vector<ResourceID> list;

        for (auto& res : resources) list.push_back(res.id);

        return list;
    }

    ResourceType getType(ResourceID id) {
        return resources.at(id).type;
    }

    uint32_t getCapabilities(ResourceID id) {
        return resources.at(id).capabilities;
    }

    Driver::BaseDriver* getCreator(ResourceID id) {
        return resources.at(id).creator;
    }

    Driver::BaseDriver* getMaintaner(ResourceID id) {
        return resources.at(id).maintaner;
    }

    void changeMaintaner(ResourceID id, Driver::BaseDriver* driver) {
        resources.at(id).maintaner = driver;
    }

    //---------------------API---------------------
}