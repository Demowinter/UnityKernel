#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <libbase/string.hpp>

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

    char* itoa(int64_t value, char* buffer, int base) {
        if (base < 2 || base > 36) {
            buffer[0] = '\0';
            return buffer;
        }

        char* ptr = buffer;
        bool isNegative = value < 0;

        if (isNegative) {
            value = -value;
        }

        do {
            int digit = value % base;
            *ptr++ = (digit < 10) ? '0' + digit : 'a' + (digit - 10);
            value /= base;
        } while (value);

        if (isNegative) {
            *ptr++ = '-';
        }

        *ptr = '\0';

        // Reverse the string
        std::reverse(buffer, ptr);

        return buffer;
    }

    char* utoa(uint64_t value, char* buffer, int base) {
        if (base < 2 || base > 36) {
            buffer[0] = '\0';
            return buffer;
        }

        char* ptr = buffer;

        do {
            int digit = value % base;
            *ptr++ = (digit < 10) ? '0' + digit : 'a' + (digit - 10);
            value /= base;
        } while (value);

        *ptr = '\0';

        // Reverse the string
        std::reverse(buffer, ptr);

        return buffer;
    }

    char* ftoa(float value, char* buffer, int precision) {
        if (precision < 0) precision = 0;

        int64_t intPart = static_cast<int64_t>(value);
        float fracPart = value - static_cast<float>(intPart);

        char* ptr = itoa(intPart, buffer, 10);

        if (precision > 0) {
            *ptr++ = '.';

            for (int i = 0; i < precision; ++i) {
                fracPart *= 10;
                int digit = static_cast<int>(fracPart);
                *ptr++ = '0' + digit;
                fracPart -= digit;
            }
        }

        *ptr = '\0';
        return buffer;
    }

    char* ltoa(int64_t value, char* buffer, int base) {
        return itoa(value, buffer, base);
    }

    char* ultoa(uint64_t value, char* buffer, int base) {
        return utoa(value, buffer, base);
    }
}