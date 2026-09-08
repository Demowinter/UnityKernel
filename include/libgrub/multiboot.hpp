#pragma once
#include <cstdint>
#include <cstddef>

namespace GRUB {
    constexpr uint32_t multibootMagic = 0x36D76289;

    enum class MultibootTagType : uint32_t {
        End = 0,
        CommandLine = 1,
        BootLoaderName = 2,
        Module = 3,
        BasicMemoryInfo = 4,
        BootDevice = 5,
        MemoryMap = 6,
        Vbe = 7,
        Framebuffer = 8,
        ElfSections = 9,
        Apm = 10,
        Efi32 = 11,
        Efi64 = 12,
        Smbios = 13,
        AcpiOld = 14,
        AcpiNew = 15,
        Network = 16,
        EfiMmap = 17,
        EfiBs = 18,
        Efi32Ih = 19,
        Efi64Ih = 20,
        LoadBaseAddr = 21
    };

    struct MultibootInfo {
        uint32_t totalSize;
        uint32_t reserved;
    };

    struct MultibootTag {
        uint32_t type;
        uint32_t size;
    };

    class MultibootTagIterator {
    public:
        MultibootTagIterator() = default;
        const MultibootTag& operator*() const;
        const MultibootTag* operator->() const;
        MultibootTagIterator& operator++();

        bool operator==(const MultibootTagIterator& other) const;
        bool operator!=(const MultibootTagIterator& other) const;

    private:
        friend class MultibootParser;
        explicit MultibootTagIterator(const MultibootTag* tag, const uint8_t* end);

        const MultibootTag* tag = nullptr;
        const uint8_t* end = nullptr;
    };

    class MultibootParser {
    public:
        explicit MultibootParser(const MultibootInfo* info);

        bool isValid() const;
        MultibootTagIterator begin() const;
        MultibootTagIterator end() const;
        const MultibootTag* findTag(MultibootTagType type) const;

    private:
        const MultibootInfo* info;
        const uint8_t* tagsEnd;
        bool valid;
    };

    bool checkMultiboot(uint32_t magic);
    bool checkMultiboot(uint32_t magic, const MultibootInfo* info);
}