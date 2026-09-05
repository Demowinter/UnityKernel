#pragma once
#include <libstd/function.hpp>

namespace STDLib {
    class ScopeGuard {
    public:
        ScopeGuard() = default;
        ScopeGuard(Function<void()>&& func) : func{std::move(func)} {}

        ~ScopeGuard() { if (func) func(); }

        void release() {
            func.clear();
        }
        
    private:
        Function<void()> func;
    };
}