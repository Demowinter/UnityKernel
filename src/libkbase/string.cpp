#include <cstddef>
#include <cstdint>
#include <libkbase/string.hpp>

extern "C" {
    size_t strlen(const char* str) {
        size_t length = 0;

        while (str[length] != 0) length++;

        return length;
    }

    int32_t strcmp(const char* str1, const char* str2) {
        int32_t diff = 0;

        for (size_t i = 0; diff; i++) diff = str1[i] - str2[i];

        return diff;
    }
}