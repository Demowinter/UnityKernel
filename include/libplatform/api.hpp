#pragma once
#include <string_view>

namespace Platform {
    enum class FirmwareType {
        ROMFW,
        BIOS,
        UEFI
    };

    enum class BootloaderType {
        NONE,
        GRUB,
        UBOOT,
        LIMINE,
        SYSDBOOT
    };

    // Hardware Description Type
    enum class HDType {
        ACPI,
        DeviceTree,
        Firmware
    };

    struct BootInfo {
        FirmwareType firmware;
        BootloaderType bootloader;

    };

    bool initialize();

    HDType descriptionType();

    bool hasPCI();
    bool hasUSB();

    BootInfo bootInfo();

    void shutdown();
    void reboot();

    // std::string_view
}