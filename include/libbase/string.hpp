#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
    size_t strlen(const char* str);
    int32_t strcmp(const char* str1, const char* str2);
}