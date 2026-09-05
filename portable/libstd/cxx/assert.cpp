#include <string_view>
#include <libenv/tty.hpp>

namespace STDLib {
    void assert(bool condition, std::string_view name) {
        if (condition) ENV::TTY::ok(name);
        else ENV::TTY::fail(name);
    }
}