#include <cstdint>

extern "C" {
    int __cxa_guard_acquire(int64_t* guardObject) {
        // lock(mutex)

        if (*guardObject) return 0;

        return 1;
    }

    void __cxa_guard_release(int64_t* guardObject) {
        // unlock(mutex)

        *guardObject = 1;
    }

    void __cxa_guard_abort(int64_t* guardObject) {
        // TODO: panic("__cxa_guard_abort()");
    }
}