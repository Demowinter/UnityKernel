#include <cstdint>
#include <algorithm>
#include <libkbase/algo.hpp>

extern "C" {
    uint64_t alignUp(uint64_t value, uint64_t alignment) {
        return value + (alignment - (value % alignment)) % alignment;
    }

    uint64_t alignDown(uint64_t value, uint64_t alignment) {
        return value - (value % alignment);
    }
    
    uint64_t alignUpPow2(uint64_t value, uint64_t alignment) {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    uint64_t alignDownPow2(uint64_t value, uint64_t alignment) {
        return value & ~(alignment - 1);
    }

    bool inRangeII(int64_t value, int64_t min, int64_t max) {
        return value >= min && value <= max;
    }

    bool inRangeEI(int64_t value, int64_t min, int64_t max) {
        return value > min && value <= max;
    }

    bool inRangeIE(int64_t value, int64_t min, int64_t max) {
        return value >= min && value < max;
    }

    bool inRangeEE(int64_t value, int64_t min, int64_t max) {
        return value > min && value < max;
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