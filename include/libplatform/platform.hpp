#pragma once
// #include <string_view>

namespace Platform {
    enum class FirmwareType {
        NONE,
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
        NONE,
        ACPI,
        DeviceTree,
        Firmware
    };

    struct Info {
        FirmwareType firmware;
        BootloaderType bootloader;
        HDType hdt;
    };

    bool initialize(const Info& info);

    bool hasUSB();

    // namespace APIC {
    //     void shutdown();
    //     void reboot();
    // }
}