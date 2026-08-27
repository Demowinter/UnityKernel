#include <kernel/console.hpp>
#include <kernel/memory.hpp>
#include <kernel/system.hpp>
#include <libenv/tty.hpp>
#include <libenv/memory.hpp>
#include <libenv/system.hpp>

namespace ENV {
    namespace TTY {
        void clear() {
            Kernel::Console::clear();
        }

        void newline() {
            Kernel::Console::newline();
        }

        void write(std::byte byte, uint8_t color) {
            Kernel::Console::write(byte, color);
        }

        void write(const void* str, size_t size, uint8_t color) {
            Kernel::Console::write(str, size, color);
        }

        void print(std::string_view str, uint8_t color) {
            Kernel::Console::print(str, color);
        }
    
        void println(std::string_view str, uint8_t color) {
            Kernel::Console::println(str, color);
        }

        void ok(std::string_view str, bool newline) {
            Kernel::Console::ok(str, newline);
        }

        void info(std::string_view str, bool newline) {
            Kernel::Console::info(str, newline);
        }
        
        void warn(std::string_view str, bool newline) {
            Kernel::Console::warn(str, newline);
        }
        
        void fail(std::string_view str, bool newline) {
            Kernel::Console::fail(str, newline);
        }
    }

    namespace Memory {
        void* allocate(size_t size) {
            return Kernel::Memory::allocate(size);
        }

        void deallocate(void* ptr) {
            Kernel::Memory::deallocate(ptr);
        }
    }

    namespace System {
        [[noreturn]] void panic(std::string_view who, std::string_view what) {
            Kernel::System::panic(who, what);
        }
    }
}