#pragma once
#include <cstddef>
#include <cstdint>

namespace ACPI {
    #pragma pack(push, 1)
    struct SystemDescriptionTable {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oemId[6];
        char oemTableId[8];
        uint32_t oemRevision;
        uint32_t creatorId;
        uint32_t creatorRevision;
    };
    #pragma pack(pop)

    bool initialize(const void* rsdp, size_t rsdpSize);
    bool isInitialized();
    const SystemDescriptionTable* rootTable();
    const SystemDescriptionTable* findTable(const char signature[4]);
}