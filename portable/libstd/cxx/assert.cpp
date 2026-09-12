#include <string_view>
#include <libenv/tty.hpp>
#include <libstd/assert.hpp>

namespace STDLib {
    void assert(bool condition, std::string_view msg) {
        if (condition) ENV::TTY::ok("Assertion passed! | ", false);
        else ENV::TTY::fail("Assertion failed! | ", false);

        ENV::TTY::println(msg);
    }
}