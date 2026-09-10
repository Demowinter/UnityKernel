#include <cstddef>
#include <libbase/memory.hpp>
#include <liblltools/cursor.hpp>

namespace LLTools {
    void Cursor::set(void* base, size_t rindex, size_t windex) {
        this->base = static_cast<uint8_t*>(base);
        this->rindex = rindex;
        this->windex = windex;
    }

    void Cursor::read(void* dest, size_t size) {
        memmove(dest, base + rindex, size);

        rindex += size;
    }

    void Cursor::peek(void* dest, size_t size) {
        memmove(dest, base + rindex, size);
    }

    void Cursor::write(const void* buff, size_t size) {
        memmove(base + windex, buff, size);

        windex += size;
    }

    void Cursor::rskip(size_t size) {
        rindex += size;
    }

    void Cursor::wskip(size_t size) {
        windex += size;
    }
}