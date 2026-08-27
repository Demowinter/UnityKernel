#pragma once
#include <cstddef>

namespace ENV::Memory {
    void* allocate(size_t size);
    void deallocate(void* ptr);
}