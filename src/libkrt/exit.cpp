#include <string_view>
#include <cstdint>
#include <libkernel/system.hpp>
#include <libkrt/krt.hpp>

using ExitFunc = void(*)(void*);

struct ExitEntry {
    ExitFunc func;
    void* param;
    
    void* dso;

    ExitEntry* next;
    ExitEntry* prev;
};

static ExitEntry* exitListEnd = nullptr;

// C++ ABI
extern "C" {
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

// Kernel Runtime API
namespace KernelRT {
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