#include <algorithm>
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
        size_t l1 = strlen(str1);
        size_t l2 = strlen(str2);

        size_t minLength = std::min(l1, l2);
        size_t maxLength = std::max(l1, l2);

        const char* longStr = (l1 > l2) ? str1 : str2;

        size_t index = 0;

        int32_t diff = 0;

        for (; index < minLength; index++) diff += str1[index] - str2[index];
        for (; index < maxLength; index++) diff += longStr[index];

        return diff;
    }
}