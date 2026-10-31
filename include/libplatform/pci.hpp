#pragma once
#include <optional>
#include <cstdint>

namespace Platform::PCI {
    // enum class Status {
    //     PRESENT,
    //     NOTPRESENT
    // };

    // class CM1 {

    // };

    // Status status();
    bool isAvailable();

    template<typename T>
    std::optional<T> readConfig(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);

    template<typename T>
    bool writeConfig(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, T value);
}