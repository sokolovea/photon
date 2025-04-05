#pragma once
#include <cstdint>
#include <cmath>
#include <complex>
#include "tiff.h"
#include "pixel.h"

using namespace std;

template<typename T>
static inline uint64_t getValueInInterval(T value, T inputLowBorder, T inputUpperBorder, uint64_t outputMaxValue) {
    if (value >= inputUpperBorder) {
        return outputMaxValue;
    }
    if (value < inputLowBorder) {
        return 0;
    }
    return ((value - inputLowBorder) * outputMaxValue + (inputUpperBorder - inputLowBorder) / 2) / (inputUpperBorder - inputLowBorder);
}

inline auto getNormalizedChannelValue(int64_t value, int64_t maxChannelValue) {
    if (value < 0) {
        value = std::abs(value);
    }
    if (value > maxChannelValue) {
        value = maxChannelValue;
    }
    return value;
}

inline uint64_t getRowSizeWithPadding(uint64_t rowSizeWithoutPadding) {
    return ((rowSizeWithoutPadding + 3) &~ 3);
}

template<typename T>
inline T divideWithCeil(T a, T b) {
    return (a + b - 1) / b;
}

template<typename T>
inline std::complex<T> divideComplexByReal(std::complex<T> complex, double real) {
    return std::complex<T>(complex.real() / real,  complex.imag() / real);
}

template<typename T>
inline T complexSquare(std::complex<T> data) {
    return data.real() * data.real() + data.imag() * data.imag();
}

inline double complexSquare(Complex16Pixel& data) {
    return data.real * data.real + data.imag * data.imag;
}
