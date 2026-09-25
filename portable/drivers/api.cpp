#include <initializer_list>
#include <libstd/vector.hpp>
#include <drivers/api.hpp>

namespace Driver {
    BaseDriver::BaseDriver(Category driCategory, Type driType, std::initializer_list<Info> deps) : driverInfo{driCategory, driType}, driverDependencies{deps} {}

    Category BaseDriver::category() const {
        return driverInfo.category;
    }

    Type BaseDriver::type() const {
        return driverInfo.type;
    }

    const STDLib::Vector<Info>& BaseDriver::dependencies() const {
        return driverDependencies;
    }
}