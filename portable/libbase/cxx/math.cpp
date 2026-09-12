#include <cstdint>
#include <libbase/math.hpp>

extern "C" {

bool isPowerOfTwo(uint64_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

uint64_t roundUpDiv(uint64_t value, uint64_t divisor) {
    return (value + divisor - 1) / divisor;
}

uint64_t roundDownDiv(uint64_t value, uint64_t divisor) {
    return value / divisor;
}

uint64_t minUint64(uint64_t a, uint64_t b) {
    return a < b ? a : b;
}

uint64_t maxUint64(uint64_t a, uint64_t b) {
    return a > b ? a : b;
}

uint64_t clampUint64(uint64_t value, uint64_t min, uint64_t max) {
    return minUint64(maxUint64(value, min), max);
}

double fabs(double x) {
    return x < 0 ? -x : x;
}

double floor(double x) {
    int64_t i = static_cast<int64_t>(x);
    double truncated = static_cast<double>(i);
    if (x < 0.0 && truncated != x) {
        truncated -= 1.0;
    }
    return truncated;
}

double ceil(double x) {
    int64_t i = static_cast<int64_t>(x);
    double truncated = static_cast<double>(i);
    if (x > 0.0 && truncated != x) {
        truncated += 1.0;
    }
    return truncated;
}

double round(double x) {
    return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

double sqrt(double x) {
    if (x <= 0.0) return 0.0;

    union { double d; uint64_t u; } bits;
    bits.d = x;
    bits.u = (bits.u >> 1) + 0x1FF7A3BEA91D9B1BULL;
    double guess = bits.d;

    for (int i = 0; i < 8; ++i) {
        guess = 0.5 * (guess + x / guess);
    }
    return guess;
}

double pow(double x, double y) {
    bool negativeExp = y < 0.0;
    int64_t n = static_cast<int64_t>(negativeExp ? -y : y);

    double result = 1.0;
    double base = x;
    while (n > 0) {
        if (n & 1) result *= base;
        base *= base;
        n >>= 1;
    }
    return negativeExp ? 1.0 / result : result;
}

double fmod(double x, double y) {
    if (y == 0.0) return 0.0;
    bool neg = x < 0.0;
    double ax = neg ? -x : x;
    double ay = y < 0.0 ? -y : y;
    if (ax < ay) return x;

    while (ax >= ay) {
        double t = ay;
        while (t * 2.0 <= ax) t *= 2.0;
        ax -= t;
    }
    return neg ? -ax : ax;
}

}