#include <string_view>
#include <cstddef>
#include <cstdint>
#include <libkernel/memory.hpp>
#include <libkernel/system.hpp>
#include <libkrt/krt.hpp>

using ConstructorFunc = void(*)();
using ExitFunc = void(*)(void*);

struct ExitEntry {
    ExitFunc func;
    void* param;
    
    void* dso;

    ExitEntry* next;
    ExitEntry* prev;
};

static ExitEntry* exitListEnd = nullptr;

// C++ symbols
extern "C" {
    extern ConstructorFunc __init_array_start[];
    extern ConstructorFunc __init_array_end[];
}

// C++ ABI
extern "C" {
    int __cxa_guard_acquire(int64_t* guardObject) {
        // lock(mutex)

        if (*guardObject) return 0;

        return 1;
    }

    void __cxa_guard_release(int64_t* guardObject) {
        *guardObject = 1;
        
        // unlock(mutex)
    }

    void __cxa_guard_abort(int64_t* guardObject) {
        KernelRT::abort("__cxa_guard_abort()", "error while constructing the object");
    }

    int __cxa_atexit(ExitFunc func, void* param, void* dso) {
        ExitEntry* entry = new ExitEntry;

        if (!entry) return 1;

        *entry = {func, param, dso};

        if (!exitListEnd) exitListEnd = entry;

        else {
            entry->prev = exitListEnd;
            exitListEnd->next = entry;

            exitListEnd = entry;
        }

        return 0;
    }

    void __cxa_finalize(void* dso) {
        for (auto entry = exitListEnd; entry != nullptr; entry = entry->prev)
                if (!dso || entry->dso == dso) entry->func(entry->param);
    }
}

// C API
extern "C" {
    [[noreturn]] void abort() {
        KernelRT::abort("C::abort()", "abnormal program termination");
    }
}

// C++ API
namespace std {
    [[noreturn]] void terminate() {
        KernelRT::abort("std::terminate()", "unrecoverable C++ runtime failure");
    }
}

// C++ memory API
void* operator new(size_t size) {
    return Kernel::Memory::allocate(size);
}

void* operator new[](size_t size) {
    return Kernel::Memory::allocate(size);
}

void operator delete(void* ptr) {
    Kernel::Memory::deallocate(ptr);
}

void operator delete(void* ptr, size_t) {
    Kernel::Memory::deallocate(ptr);
}

void operator delete[](void* ptr) {
    Kernel::Memory::deallocate(ptr);
}

void operator delete[](void* ptr, size_t) {
    Kernel::Memory::deallocate(ptr);
}

// Kernel Runtime API
namespace KernelRT {
    void initialize() {
        for (auto ctor = __init_array_start; ctor != __init_array_end; ctor++) (*ctor)();
    }
    
    void finalize() {
        __cxa_finalize(nullptr);
    }

    [[noreturn]] void abort(std::string_view what) {
        abort("KernelRT::abort()", what);
    }

    [[noreturn]] void abort(std::string_view who, std::string_view what) {
        static bool aborted = false;

        if (!aborted) {
            aborted = true;
            
            finalize();
        }

        Kernel::System::panic(who, what);
    }
}