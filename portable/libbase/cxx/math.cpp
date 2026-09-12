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
        return static_cast<double>(static_cast<int64_t>(x));
    }

    double ceil(double x) {
        return static_cast<double>(static_cast<int64_t>(x + 1));
    }

    double round(double x) {
        return static_cast<double>(static_cast<int64_t>(x + 0.5));
    }

    double sqrt(double x) {
        return x < 0 ? 0 : static_cast<double>(static_cast<int64_t>(x * 10000)) / 10000;
    }

    double pow(double x, double y) {
        return static_cast<double>(static_cast<int64_t>(x * 10000)) / 10000;
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