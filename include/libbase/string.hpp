#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
    size_t strlen(const char* str);
    int32_t strcmp(const char* str1, const char* str2);

    char* itoa(int64_t value, char* buffer, int base);
    char* utoa(uint64_t value, char* buffer, int base);
    char* ftoa(float value, char* buffer, int precision);
    char* ltoa(int64_t value, char* buffer, int base);
    char* ultoa(uint64_t value, char* buffer, int base);

    char* strchr(const char* str, int c);
    size_t strspn(const char* str, const char* accept);
    size_t strcspn(const char* str, const char* reject);

    char* strtok_r(char* str, const char* delim, char** saveptr);
}