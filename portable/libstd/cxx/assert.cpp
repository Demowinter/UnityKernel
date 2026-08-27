#include <string_view>
#include <kernel/console.hpp>

namespace STDLib {
    void assert(bool condition, std::string_view name) {
        if (condition) Kernel::Console::ok(name);
        else Kernel::Console::fail(name);
    }
}