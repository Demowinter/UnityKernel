#pragma once
#include <libstd/vector.hpp>
#include <drivers/api.hpp>

namespace Kernel::DriverSubsystem {
    void initialize();
    void finalize();

    Driver::BaseDriver* loadDriver(Driver::Category category, Driver::Type type);
    void unloadDriver(Driver::BaseDriver* driver);

    const STDLib::Vector<Driver::BaseDriver*> getLoadedDrivers();
    const STDLib::Vector<Driver::BaseDriver*> getLoadedDrivers(Driver::Category category, Driver::Type type);

    bool isDriverLoaded(Driver::BaseDriver* driver);
}