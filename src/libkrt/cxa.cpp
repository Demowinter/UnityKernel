using DestructorFunc = void(*)(void*);

extern "C" {
    // extern void* __dso_handle = &__dso_handle;

    int __cxa_atexit(DestructorFunc func, void* object, void* dso) {
        return 0;
    }
}