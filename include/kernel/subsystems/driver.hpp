#pragma once
#include <limits>
#include <cstddef>
#include <libstd/vector.hpp>
#include <drivers/driver.hpp>

namespace Kernel::DriverSubsystem {
    using DriverID = std::size_t;

    constexpr DriverID invalidID = std::numeric_limits<DriverID>::max();

    void initialize();
    void finalize();

    bool loadDriver(DriverID id);
    void unloadDriver(DriverID id);

    STDLib::Vector<DriverID> getDrivers(Driver::Category category = Driver::Category::ANY, Driver::Type type = Driver::Type::ANY);
    STDLib::Vector<DriverID> getLoadedDrivers(Driver::Category category = Driver::Category::ANY, Driver::Type type = Driver::Type::ANY);

    bool isDriverLoaded(DriverID id);

    Driver::Info getDriverInfo(DriverID id);
}