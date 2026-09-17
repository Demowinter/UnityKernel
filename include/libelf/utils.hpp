#pragma once
#include <libelf/header.hpp>

namespace ELF::Utils {
    void dump(const ELFHeader& header);
}