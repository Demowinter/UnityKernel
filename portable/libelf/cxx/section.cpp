#include <cstddef>
#include <cstdint>
#include <liblltools/cursor.hpp>
#include <libenv/system.hpp>
#include <libelf/header.hpp>
#include <libelf/parser.hpp>
#include <libelf/section.hpp>

namespace ELF {
    ELFSectionIterator::ELFSectionIterator(const ELFHeader& header, void* addr) : header{header}, addr{addr} {}

    ELFSectionEntry ELFSectionIterator::operator*() const {
        return parseSectionEntry(addr, header);
    }

    ELFSectionEntry ELFSectionIterator::operator->() const {
        return parseSectionEntry(addr, header);
    }

    ELFSectionIterator& ELFSectionIterator::operator++() {
        addr = static_cast<uint8_t*>(addr) + header.shentsize;

        return *this;
    }

    bool ELFSectionIterator::operator==(const ELFSectionIterator& other) const {
        return addr == other.addr;
    }

    bool ELFSectionIterator::operator!=(const ELFSectionIterator& other) const {
        return addr != other.addr;
    }

    ELFSectionTable::ELFSectionTable(const ELFHeader& header, void* baseAddr) : header{header} {
        startAddr = static_cast<uint8_t*>(baseAddr) + header.shoff;
        endAddr = static_cast<uint8_t*>(startAddr) + header.shnum * header.shentsize;
    }

    ELFSectionEntry ELFSectionTable::at(size_t index) {
        void* entryAddr = static_cast<uint8_t*>(startAddr) + header.shentsize * index;

        if (entryAddr >= endAddr) ENV::System::panic("ELFSectionTable::at(size_t)", "Invalid index");

        return parseSectionEntry(entryAddr, header);
    }

    ELFSectionIterator ELFSectionTable::begin() {
        return ELFSectionIterator{header, startAddr};
    }

    ELFSectionIterator ELFSectionTable::end() {
        return ELFSectionIterator{header, endAddr};
    }
}