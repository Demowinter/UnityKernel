#include <cstdint>
#include <libkrt/krt.hpp>

using ExitFunc = void(*)(void*);

struct ExitEntry {
    ExitFunc func;
    void* param;
    
    void* dso;

    ExitEntry* next;
    ExitEntry* prev;
};

ExitEntry* exitListEnd = nullptr;

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
            if (entry->dso == dso) entry->func(entry->param);
    }

    void abort() {
        KernelRT::finalize();
    }
}

namespace KernelRT {
    void finalizeAll() {
        for (auto entry = exitListEnd; entry != nullptr; entry = entry->prev)
            entry->func(entry->param);
    }

    void finalize() {
        finalizeAll();
    }
}