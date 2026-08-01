#pragma onces
#include <cstdint>

namespace Kernel::Memory {
    void* allocate(uint64_t size);
    void deallocate(void* ptr);
}