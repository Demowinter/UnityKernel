#include <cstdint>
#include <libgrub/multiboot.hpp>

namespace GRUB {
    namespace {
        constexpr size_t tagAlignment = 8;
        constexpr size_t minimumInfoSize = sizeof(MultibootInfo) + sizeof(MultibootTag);

        size_t alignedTagSize(uint32_t size) {
            return (static_cast<size_t>(size) + tagAlignment - 1) & ~(tagAlignment - 1);
        }
    }

    MultibootTagIterator::MultibootTagIterator(const MultibootTag* tag, const uint8_t* end)
        : tag(tag), end(end) {}

    const MultibootTag& MultibootTagIterator::operator*() const {
        return *tag;
    }

    const MultibootTag* MultibootTagIterator::operator->() const {
        return tag;
    }

    MultibootTagIterator& MultibootTagIterator::operator++() {
        if (tag == nullptr || tag->size < sizeof(MultibootTag)) {
            tag = nullptr;
            return *this;
        }

        size_t size = alignedTagSize(tag->size);
        const uint8_t* next = reinterpret_cast<const uint8_t*>(tag) + size;
        tag = next < end ? reinterpret_cast<const MultibootTag*>(next) : nullptr;
        return *this;
    }

    bool MultibootTagIterator::operator==(const MultibootTagIterator& other) const {
        return tag == other.tag;
    }

    bool MultibootTagIterator::operator!=(const MultibootTagIterator& other) const {
        return !(*this == other);
    }

    MultibootParser::MultibootParser(const MultibootInfo* info)
        : info(info), tagsEnd(nullptr), valid(false) {
        if (info == nullptr || info->totalSize < minimumInfoSize) return;

        const uint8_t* start = reinterpret_cast<const uint8_t*>(info);
        tagsEnd = start + info->totalSize;
        const uint8_t* cursor = start + sizeof(MultibootInfo);

        while (cursor + sizeof(MultibootTag) <= tagsEnd) {
            const auto* tag = reinterpret_cast<const MultibootTag*>(cursor);
            if (tag->size < sizeof(MultibootTag)) return;

            size_t size = alignedTagSize(tag->size);
            if (size < tag->size || cursor + size > tagsEnd) return;

            cursor += size;
            if (tag->type == static_cast<uint32_t>(MultibootTagType::End)) {
                valid = tag->size == sizeof(MultibootTag) && cursor == tagsEnd;
                return;
            }
        }
    }

    bool MultibootParser::isValid() const {
        return valid;
    }

    MultibootTagIterator MultibootParser::begin() const {
        if (!valid) return end();
        const auto* first = reinterpret_cast<const MultibootTag*>(
            reinterpret_cast<const uint8_t*>(info) + sizeof(MultibootInfo));
        return MultibootTagIterator(first, tagsEnd);
    }

    MultibootTagIterator MultibootParser::end() const {
        return MultibootTagIterator(nullptr, tagsEnd);
    }

    const MultibootTag* MultibootParser::findTag(MultibootTagType type) const {
        const uint32_t wantedType = static_cast<uint32_t>(type);
        for (auto iterator = begin(); iterator != end(); ++iterator) {
            if (iterator->type == wantedType) return &*iterator;
        }
        return nullptr;
    }

    bool checkMultiboot(uint32_t magic) {
        return magic == multibootMagic;
    }

    bool checkMultiboot(uint32_t magic, const MultibootInfo* info) {
        return checkMultiboot(magic) && MultibootParser(info).isValid();
    }
}
