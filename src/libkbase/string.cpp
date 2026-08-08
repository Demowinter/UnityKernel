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

        for (size_t i = 0; diff; i++) {
            char ch1 = str1[i];
            char ch2 = str2[i];

            if (!ch1) return -1;
            if (!ch2) return 1;

            diff = ch1 - ch2;
        }

        return diff;
    }
}