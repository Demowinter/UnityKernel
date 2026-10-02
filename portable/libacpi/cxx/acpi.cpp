#include <cstdint>
#include <libacpi/acpi.hpp>

namespace Driver::ACPI {
    namespace {
        #pragma pack(push, 1)
        struct RSDPDescriptor {
            char signature[8];
            uint8_t checksum;
            char oemId[6];
            uint8_t revision;
            uint32_t rsdtAddress;
        };

        struct RSDPDescriptor20 {
            RSDPDescriptor firstPart;
            uint32_t length;
            uint64_t xsdtAddress;
            uint8_t extendedChecksum;
            uint8_t reserved[3];
        };
        #pragma pack(pop)

        const SystemDescriptionTable* root = nullptr;
        bool initialized = false;

        bool checksumValid(const void* data, size_t length) {
            const auto* bytes = static_cast<const uint8_t*>(data);
            uint8_t sum = 0;
            for (size_t i = 0; i < length; ++i) sum += bytes[i];
            return sum == 0;
        }

        bool signatureEquals(const char* left, const char* right, size_t length) {
            for (size_t i = 0; i < length; ++i) {
                if (left[i] != right[i]) return false;
            }
            return true;
        }

        bool validTable(const SystemDescriptionTable* table) {
            return table != nullptr &&
                table->length >= sizeof(SystemDescriptionTable) &&
                checksumValid(table, table->length);
        }

        const SystemDescriptionTable* tableAt(uint64_t address) {
            if (address == 0 || address > static_cast<uint64_t>(~uintptr_t{0})) return nullptr;
            return reinterpret_cast<const SystemDescriptionTable*>(static_cast<uintptr_t>(address));
        }

        uint64_t readEntryAddress(const uint8_t* entry, size_t size) {
            uint64_t address = 0;
            for (size_t i = 0; i < size; ++i) {
                address |= static_cast<uint64_t>(entry[i]) << (i * 8);
            }
            return address;
        }
    }

    bool initialize(const void* rsdpData, size_t rsdpSize) {
        root = nullptr;
        initialized = false;

        if (rsdpData == nullptr || rsdpSize < sizeof(RSDPDescriptor)) return false;

        const auto* rsdp = static_cast<const RSDPDescriptor*>(rsdpData);
        if (!signatureEquals(rsdp->signature, "RSD PTR ", sizeof(rsdp->signature)) ||
            !checksumValid(rsdp, sizeof(RSDPDescriptor))) {
            return false;
        }

        const SystemDescriptionTable* candidate = nullptr;
        bool isXsdt = false;

        if (rsdp->revision >= 2) {
            if (rsdpSize < sizeof(RSDPDescriptor20)) return false;

            const auto* rsdp20 = static_cast<const RSDPDescriptor20*>(rsdpData);
            if (rsdp20->length < sizeof(RSDPDescriptor20) ||
                rsdp20->length > rsdpSize ||
                !checksumValid(rsdpData, rsdp20->length)) {
                return false;
            }

            candidate = tableAt(rsdp20->xsdtAddress);
            isXsdt = candidate != nullptr;
        }

        if (candidate == nullptr) candidate = tableAt(rsdp->rsdtAddress);
        if (!validTable(candidate)) return false;

        const char* expectedSignature = isXsdt ? "XSDT" : "RSDT";
        if (!signatureEquals(candidate->signature, expectedSignature, 4)) return false;

        const uint32_t entrySize = isXsdt ? sizeof(uint64_t) : sizeof(uint32_t);
        if ((candidate->length - sizeof(SystemDescriptionTable)) % entrySize != 0) return false;

        root = candidate;
        initialized = true;
        return true;
    }

    bool isInitialized() {
        return initialized;
    }

    const SystemDescriptionTable* rootTable() {
        return root;
    }

    const SystemDescriptionTable* findTable(const char signature[4]) {
        if (!initialized || signature == nullptr) return nullptr;

        const bool isXsdt = signatureEquals(root->signature, "XSDT", 4);
        const uint32_t entrySize = isXsdt ? sizeof(uint64_t) : sizeof(uint32_t);
        const auto* entries = reinterpret_cast<const uint8_t*>(root) + sizeof(SystemDescriptionTable);
        const uint32_t entryCount = (root->length - sizeof(SystemDescriptionTable)) / entrySize;

        for (uint32_t i = 0; i < entryCount; ++i) {
            const uint64_t address = readEntryAddress(entries + i * entrySize, entrySize);
            const auto* table = tableAt(address);
            if (validTable(table) && signatureEquals(table->signature, signature, 4)) return table;
        }
        return nullptr;
    }
}