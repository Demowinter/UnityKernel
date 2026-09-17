#pragma once
#include <cstddef>
#include <libelf/header.hpp>

namespace ELF {
    class ELFSegmentIterator {
    public:
        ELFSegmentIterator(void* addr, ELFHeader header);

        ELFSegmentEntry operator*() const;
        ELFSegmentEntry operator->() const;

        ELFSegmentIterator& operator++();

        bool operator==(const ELFSegmentIterator& other) const;
        bool operator!=(const ELFSegmentIterator& other) const;

    private:
        ELFHeader header;

        void* addr;
    };

    class ELFSegmentTable {
    public:
        ELFSegmentTable(ELFHeader header, void* baseAddr);

        ELFSegmentEntry at(size_t index);

        ELFSegmentIterator begin();
        ELFSegmentIterator end();

    private:
        ELFHeader header;

        void* startAddr;
        void* endAddr;
    };
}