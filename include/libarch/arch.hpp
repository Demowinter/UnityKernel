#pragma once

namespace Arch {
    enum class Type {
        x86,
        x86_64,
        ARM32,
        ARM64
    };

    void initialize();
}