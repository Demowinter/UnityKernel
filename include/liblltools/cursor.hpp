#pragma once
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace LLTools {
    class Cursor {
    public:
        Cursor() = default;
        Cursor(void* base, size_t rindex = 0, size_t windex = 0) : base{static_cast<uint8_t*>(base)}, rindex{rindex}, windex{windex} {}

        void set(void* base, size_t rindex, size_t windex);

        void read(void* dest, size_t size);
        void peek(void* dest, size_t size);

        void write(const void* buff, size_t size);

        void rskip(size_t size);
        void wskip(size_t size);

        template<typename T>
        T read() {
            T obj{};

            read(&obj, sizeof(T));

            return obj;
        }

        template<typename T>
        T peek() {
            T obj{};

            peek(&obj, sizeof(T));

            return obj;
        }

        template<typename T>
        void write(const T& obj) {
            write(&obj, sizeof(T));
        }

    private:
        uint8_t* base;

        size_t rindex;
        size_t windex;
    };

    static_assert(std::is_trivially_constructible_v<Cursor>);
    static_assert(std::is_trivially_destructible_v<Cursor>);
}