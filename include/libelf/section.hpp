#pragma once
#include <cstddef>
#include <libelf/header.hpp>

namespace ELF {
    class ELFSectionIterator {
    public:
        ELFSectionIterator(const ELFHeader& header, void* addr);

        ELFSectionEntry operator*() const;
        ELFSectionEntry operator->() const;

        ELFSectionIterator& operator++();

        bool operator==(const ELFSectionIterator& other) const;
        bool operator!=(const ELFSectionIterator& other) const;

    private:
        const ELFHeader& header;

        void* addr;
    };

    class ELFSectionTable {
    public:
        ELFSectionTable(const ELFHeader& header, void* baseAddr);

        ELFSectionEntry at(size_t index);

        ELFSectionIterator begin();
        ELFSectionIterator end();

    private:
        const ELFHeader& header;

        void* startAddr;
        void* endAddr;
    };
}