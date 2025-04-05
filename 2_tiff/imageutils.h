#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>

using namespace std;

struct ColorsConstraits {
    std::pair<uint64_t, uint64_t> red;
    std::pair<uint64_t, uint64_t> green;
    std::pair<uint64_t, uint64_t> blue;
    friend std::ostream& operator<<(std::ostream& os, const ColorsConstraits& obj);
};

std::ostream& operator<<(std::ostream& os, const ColorsConstraits& obj) {
    os  << "Red sample: left border: " << obj.red.first << "; right border = " << obj.red.second << "" << std::endl \
        << "Green sample: left border: " << obj.green.first << "; right border = " << obj.green.second << "" << std::endl \
        << "Blue sample: left border: " << obj.blue.first << "; right border = " << obj.blue.second << "";
    return os;
}

static inline uint64_t getValueInInterval(uint64_t value, uint64_t inputLowBorder, uint64_t inputUpperBorder, uint64_t outputMaxValue) {
    if (value >= inputUpperBorder) {
        return outputMaxValue;
    }
    if (value < inputLowBorder) {
        return 0;
    }
    return ((value - inputLowBorder) * outputMaxValue) / (inputUpperBorder - inputLowBorder);
}

template<typename T1, typename T2>
static void convertPixelFormat(T1& inputPixel, T2& outputPixel, uint64_t outputMaxValue,
                                const ColorsConstraits& colorsConstraits) {
    outputPixel.red = getValueInInterval(inputPixel.red, colorsConstraits.red.first, colorsConstraits.red.second, outputMaxValue);
    outputPixel.green = getValueInInterval(inputPixel.green, colorsConstraits.green.first, colorsConstraits.green.second, outputMaxValue);
    outputPixel.blue = getValueInInterval(inputPixel.blue, colorsConstraits.blue.first, colorsConstraits.blue.second, outputMaxValue);
}


// important: inputRowBytesCount, outputRowBytesCount - row sizes without padding
template<typename T1, typename T2, typename Tfunc>
static void decreaseResolution(T1* inputRow, T2* outputRow, uint64_t inputRowPixelsCount, uint64_t outputRowPixelsCount,
                               uint64_t coeff, Tfunc pixelConvertCallbackFunc) {
    for (uint64_t i = 0, j = 0; i < outputRowPixelsCount; i += 1, j += coeff) {
        pixelConvertCallbackFunc(inputRow[j], outputRow[i]);
        if (i >= outputRowPixelsCount || j >= inputRowPixelsCount) {
            cerr << "decreaseResolution" << endl;
        }
    }
}

inline uint64_t getRowSizeWithPadding(uint64_t rowSizeWithoutPadding) {
    return ((rowSizeWithoutPadding + 3) &~ 3);
}

template<typename T>
inline T divideWithCeil(T a, T b) {
    return (a + b - 1) / b;
}
