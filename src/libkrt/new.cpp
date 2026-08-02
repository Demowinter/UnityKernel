#include <cstddef>
#include <libkernel/memory.hpp>

void* operator new(size_t size) {
    return Kernel::Memory::allocate(size);
}

void* operator new[](size_t size) {
    return Kernel::Memory::allocate(size);
}

void operator delete(void* ptr) {
    Kernel::Memory::deallocate(ptr);
}

void operator delete[](void* ptr) {
    Kernel::Memory::deallocate(ptr);
}