#include <cstddef>
#include <cstdint>
#include <liblltools/cursor.hpp>
#include <libelf/header.hpp>
#include <libelf/parser.hpp>
#include <libelf/segment.hpp>

namespace ELF {
    ELFSegmentIterator::ELFSegmentIterator(void* addr, ELFHeader header) : header{header}, addr{addr} {}

    ELFSegmentEntry ELFSegmentIterator::operator*() const {
        return parseSegmentEntry(addr, header);
    }

    ELFSegmentEntry ELFSegmentIterator::operator->() const {
        return parseSegmentEntry(addr, header);
    }

    ELFSegmentIterator& ELFSegmentIterator::operator++() {
        addr = static_cast<uint8_t*>(addr) + header.phentsize;

        return *this;
    }

    bool ELFSegmentIterator::operator==(const ELFSegmentIterator& other) const {
        return addr == other.addr;
    }

    bool ELFSegmentIterator::operator!=(const ELFSegmentIterator& other) const {
        return addr != other.addr;
    }

    ELFSegmentTable::ELFSegmentTable(ELFHeader header, void* baseAddr) : header{header} {
        startAddr = static_cast<uint8_t*>(baseAddr) + header.phoff;
        endAddr = static_cast<uint8_t*>(startAddr) + header.phnum * header.phentsize;
    }

    ELFSegmentEntry ELFSegmentTable::at(size_t index) {
        return parseSegmentEntry(static_cast<uint8_t*>(startAddr) + header.phentsize * index, header);
    }

    ELFSegmentIterator ELFSegmentTable::begin() {
        return ELFSegmentIterator{startAddr, header};
    }

    ELFSegmentIterator ELFSegmentTable::end() {
        return ELFSegmentIterator{endAddr, header};
    }
}