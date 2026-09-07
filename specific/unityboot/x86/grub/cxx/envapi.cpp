#include <unityboot/console.hpp>
#include <unityboot/memory.hpp>
#include <unityboot/system.hpp>
#include <libenv/tty.hpp>
#include <libenv/memory.hpp>
#include <libenv/system.hpp>

namespace ENV {
    namespace TTY {
        void clear() {
            UnityBoot::Console::clear();
        }

        void newline() {
            UnityBoot::Console::newline();
        }

        void write(std::byte byte, uint8_t color) {
            UnityBoot::Console::write(byte, color);
        }

        void write(const void* str, size_t size, uint8_t color) {
            UnityBoot::Console::write(str, size, color);
        }

        void print(std::string_view str, uint8_t color) {
            UnityBoot::Console::print(str, color);
        }
    
        void println(std::string_view str, uint8_t color) {
            UnityBoot::Console::println(str, color);
        }

        void ok(std::string_view str, bool newline) {
            UnityBoot::Console::ok(str, newline);
        }

        void info(std::string_view str, bool newline) {
            UnityBoot::Console::info(str, newline);
        }
        
        void warn(std::string_view str, bool newline) {
            UnityBoot::Console::warn(str, newline);
        }
        
        void fail(std::string_view str, bool newline) {
            UnityBoot::Console::fail(str, newline);
        }
    }

    namespace Memory {
        void* allocate(size_t size) {
            return UnityBoot::Memory::allocate(size);
        }

        void deallocate(void* ptr) {
            UnityBoot::Memory::deallocate(ptr);
        }
    }

    namespace System {
        [[noreturn]] void panic(std::string_view who, std::string_view what) {
            UnityBoot::System::panic(who, what);
        }
    }
}