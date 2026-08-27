#include <uniboot/console.hpp>
#include <uniboot/memory.hpp>
#include <uniboot/system.hpp>
#include <libenv/tty.hpp>
#include <libenv/memory.hpp>
#include <libenv/system.hpp>

namespace ENV {
    namespace TTY {
        void clear() {
            UniBoot::Console::clear();
        }

        void newline() {
            UniBoot::Console::newline();
        }

        void write(std::byte byte, uint8_t color) {
            UniBoot::Console::write(byte, color);
        }

        void write(const void* str, size_t size, uint8_t color) {
            UniBoot::Console::write(str, size, color);
        }

        void print(std::string_view str, uint8_t color) {
            UniBoot::Console::print(str, color);
        }
    
        void println(std::string_view str, uint8_t color) {
            UniBoot::Console::println(str, color);
        }

        void ok(std::string_view str, bool newline) {
            UniBoot::Console::ok(str, newline);
        }

        void info(std::string_view str, bool newline) {
            UniBoot::Console::info(str, newline);
        }
        
        void warn(std::string_view str, bool newline) {
            UniBoot::Console::warn(str, newline);
        }
        
        void fail(std::string_view str, bool newline) {
            UniBoot::Console::fail(str, newline);
        }
    }

    namespace Memory {
        void* allocate(size_t size) {
            // return UniBoot::Memory::allocate(size);
            return nullptr;
        }

        void deallocate(void* ptr) {
            // UniBoot::Memory::deallocate(ptr);
        }
    }

    namespace System {
        [[noreturn]] void panic(std::string_view who, std::string_view what) {
            UniBoot::System::panic(who, what);
        }
    }
}