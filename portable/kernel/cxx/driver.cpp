#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <libstd/vector.hpp>
#include <drivers/api.hpp>
#include <drivers/loadlist.hpp>
#include <kernel/subsystems/driver.hpp>

namespace Kernel::DriverSubsystem {
    static STDLib::Vector<Driver::BaseDriver*> drivers;
    static STDLib::Vector<Driver::BaseDriver*> loadedDrivers;

    constexpr uint8_t excludeNoneFlag = 0x01;
    constexpr uint8_t excludeLoadedFlag = 0x02;
    constexpr uint8_t excludeUnloadedFlag = 0x04;

    Driver::BaseDriver* findDriver(Driver::Category category, Driver::Type type, uint8_t excludeFlag, const STDLib::Vector<Driver::BaseDriver*>& excludeList = {}) {
        auto it = std::find_if(drivers.begin(), drivers.end(), [category, type, excludeFlag, &excludeList](Driver::BaseDriver* driver) {
            bool category_ok = driver->category() == category;
            bool type_ok = driver->type() == type || type == Driver::Type::ANY;
            bool loaded = isDriverLoaded(driver);
            bool excludeVerdict = excludeFlag & excludeNoneFlag || (excludeFlag & excludeLoadedFlag && !loaded)  || (excludeFlag & excludeUnloadedFlag && loaded);
            bool excludeListVerdict = std::find(excludeList.begin(), excludeList.end(), driver) == excludeList.end();

            return category_ok && type_ok && excludeVerdict && excludeListVerdict;
        });

        return (it != drivers.end()) ? *it : nullptr;
    }

    STDLib::Vector<Driver::BaseDriver*> findDrivers(Driver::Category category, Driver::Type type, uint8_t excludeFlag, const STDLib::Vector<Driver::BaseDriver*>& excludeList = {}) {
        STDLib::Vector<Driver::BaseDriver*> result;
        STDLib::Vector<Driver::BaseDriver*> exclude = excludeList;

        while (true) {
            Driver::BaseDriver* driver = findDriver(category, type, excludeFlag, exclude);

            if (!driver) break;

            result.push_back(driver);
            exclude.push_back(driver);
        }

        return result;
    }

    bool loadInitDriver(Driver::BaseDriver* driver) {
        if (!driver) return;

        loadedDrivers.push_back(driver);

        for (const auto& dep : driver->dependencies()) {
            auto depDrivers = findDrivers(dep.category, dep.type, excludeNoneFlag);

            for (auto depDri : depDrivers) {
                if (driver->canSatisfy(depDri) && !isDriverLoaded(depDri))
                    if (!loadInitDriver(depDri)) continue;

                driver->useDriver(depDri);
            }
        }

        return driver->initialize();
    }

    //----------------------API----------------------
    void initialize() {
        drivers = Driver::loadDriverList();
    }

    void finalize() {
        for (auto driver : drivers) delete driver;

        drivers.clear();
    }

    Driver::BaseDriver* loadDriver(Driver::Category category, Driver::Type type) {
        Driver::BaseDriver* driver = findDriver(category, type, true);

        return loadInitDriver(driver) ? driver : nullptr;
    }

    void unloadDriver(Driver::BaseDriver* driver) {
        auto it = std::find(loadedDrivers.begin(), loadedDrivers.end(), driver);

        if (it != loadedDrivers.end()) {
            driver->finalize();

            loadedDrivers.erase(it);
        }
    }

    const STDLib::Vector<Driver::BaseDriver*> getLoadedDrivers() {
        return loadedDrivers;
    }

    const STDLib::Vector<Driver::BaseDriver*> getLoadedDrivers(Driver::Category category, Driver::Type type) {
        return findDrivers(category, type, excludeNoneFlag);
    }

    bool isDriverLoaded(Driver::BaseDriver* driver) {
        return std::find(loadedDrivers.begin(), loadedDrivers.end(), driver) != loadedDrivers.end();
    }
    //----------------------API----------------------
}