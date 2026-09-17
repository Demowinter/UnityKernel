#pragma once
#include <cstddef>
#include <libelf/header.hpp>

namespace ELF {
    class ELFSegmentIterator {
    public:
        ELFSegmentIterator(const ELFHeader& header, void* addr);

        ELFSegmentEntry operator*() const;
        ELFSegmentEntry operator->() const;

        ELFSegmentIterator& operator++();

        bool operator==(const ELFSegmentIterator& other) const;
        bool operator!=(const ELFSegmentIterator& other) const;

    private:
        const ELFHeader& header;

        void* addr;
    };

    class ELFSegmentTable {
    public:
        ELFSegmentTable(const ELFHeader& header, void* baseAddr);

        ELFSegmentEntry at(size_t index);

        ELFSegmentIterator begin();
        ELFSegmentIterator end();

    private:
        const ELFHeader& header;

        void* startAddr;
        void* endAddr;
    };
}