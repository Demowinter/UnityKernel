#pragma once
#include <libstd/string.hpp>

namespace Arch::CPU {
    STDLib::String manufacturer();

    void halt();
}