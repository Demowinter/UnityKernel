#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <libstd/vector.hpp>
#include <libstd/map.hpp>
#include <drivers/driver.hpp>
#include <drivers/loadlist.hpp>
#include <kernel/subsystems/driver.hpp>

namespace Kernel::DriverSubsystem {
    struct DriverEntry {
        bool loaded = false;

        Driver::BaseDriver* instance = nullptr;
    };

    static DriverID nextID = 0;
    static STDLib::Vector<DriverID> freeIDs;

    static STDLib::Map<DriverID, DriverEntry> drivers;

    constexpr uint8_t excludeNoneFlag = 0x01;
    constexpr uint8_t excludeLoadedFlag = 0x02;
    constexpr uint8_t excludeUnloadedFlag = 0x04;
    

    DriverID allocDriverID() {
        DriverID id = invalidID;

        if (freeIDs.size()) {
            id = freeIDs.back();
            freeIDs.pop_back();
        }

        else if (nextID != invalidID) id = nextID++;

        return id;
    }

    void freeDriverID(DriverID id) {
        if (std::find(freeIDs.begin(), freeIDs.end(), id) == freeIDs.end()) freeIDs.push_back(id);
    }

    STDLib::Vector<DriverID> findDrivers(Driver::Category category, Driver::Type type, uint8_t excludeFlag) {
        STDLib::Vector<DriverID> drilist;

        for (const auto& [id, driver] : drivers) {
            Driver::Info driinfo = driver.instance->info();
            
            bool category_ok = driinfo.category == category || category == Driver::Category::ANY;
            bool type_ok = driinfo.type == type || type == Driver::Type::ANY;
            bool exclude_ok = excludeFlag & excludeNoneFlag || (excludeFlag & excludeLoadedFlag && !driver.loaded)  || (excludeFlag & excludeUnloadedFlag && driver.loaded);

            if (category_ok && type_ok && exclude_ok) drilist.push_back(id);
        }

        return drilist;
    }

    //----------------------API----------------------

    void initialize() {
        for (const auto& driver : Driver::loadDriverList()) drivers.insert(allocDriverID(), DriverEntry{false, driver});
    }

    void finalize() {
        for (auto& [id, driver] : drivers) drivers.at(id).instance->finalize();
        for (auto& [id, driver] : drivers) delete driver.instance;

        drivers.clear();
    }

    bool loadDriver(DriverID id) {
        if (id == invalidID) return false;
        if (isDriverLoaded(id)) return true;

        auto& driver = drivers.at(id);

        if (driver.instance->initialize()) {
            driver.loaded = true;

            return true;
        }

        driver.instance->finalize();
        
        return false;
    }

    void unloadDriver(DriverID id) {
        auto& driver = drivers.at(id);

        driver.instance->finalize();
        driver.loaded = false;
    }

    STDLib::Vector<DriverID> getDrivers(Driver::Category category, Driver::Type type) {
        return findDrivers(category, type, excludeNoneFlag);
    }

    STDLib::Vector<DriverID> getLoadedDrivers(Driver::Category category, Driver::Type type) {
        return findDrivers(category, type, excludeUnloadedFlag);
    }

    bool isDriverLoaded(DriverID id) {
        return drivers.at(id).loaded;
    }

    Driver::Info getDriverInfo(DriverID id) {
        return drivers.at(id).instance->info();
    }
    
    //----------------------API----------------------
}