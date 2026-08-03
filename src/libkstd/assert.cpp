#include <string_view>
#include <libkernel/console.hpp>

namespace KernelSTD {
    void assert(bool condition, std::string_view name) {
        if (condition) Kernel::Console::ok(name);
        else Kernel::Console::fail(name);
    }
}