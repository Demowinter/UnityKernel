#pragma once
#include <string_view>

namespace Platform {
    // Hardware Description Provider
    enum class HDType {
        ACPI,
        DeviceTree,
        Bootloader
    };

    void initialize();

    HDType descriptionType();

    bool hasPCI();
    bool hasUSB();

    void bootInfo();

    void shutdown();
    void reboot();

    // std::string_view
}