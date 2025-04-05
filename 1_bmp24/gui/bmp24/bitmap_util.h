#pragma once
#include <cstdint>
#include <iostream>
#include <QString>
#include "bitmap.h"
#include "operationstatus.h"

using namespace std;

//T - type of pixel (bmp24, ...)

// important: inputRowBytesCount, outputRowBytesCount - row sizes without padding 
template<typename T>
static void increaseResolution(T* inputRow, T* outputRow, uint64_t inputRowPixelsCount, uint64_t outputRowPixelsCount, uint64_t coeff) {
    for (uint64_t i = 0; i < inputRowPixelsCount; i++) {
        for (uint64_t j = 0; j < coeff; j++) {
            outputRow[i * coeff + j] = inputRow[i];
            if (i * coeff + j >= outputRowPixelsCount || i >= inputRowPixelsCount) {
                cerr << "BUG increaseResolution" << endl;
            }
        }
    }
}

// important: inputRowBytesCount, outputRowBytesCount - row sizes without padding
template<typename T>
static void decreaseResolution(T* inputRow, T* outputRow, uint64_t inputRowPixelsCount, uint64_t outputRowPixelsCount, uint64_t coeff) {
    for (uint64_t i = 0, j = 0; i < outputRowPixelsCount; i += 1, j += coeff) {
        outputRow[i] = inputRow[j];
        if (i >= outputRowPixelsCount || j >= inputRowPixelsCount) {
            cerr << "decreaseResolution" << endl;
        }
    }
}

OperationStatus readImageHeadersToStructure(const string& fileName, Bitmap24Image& bitmap24Image);

OperationStatus readImagePixelsToBuffer(const string& fileName, uint8_t* outputBuffer, Bitmap24Image& bitmap24Image);

OperationStatus writeImageStructureToFile(const Bitmap24Image& bitmap24Image, const uint8_t* inputBuffer, const std::string& fileName);

inline uint64_t getRowSizeWithPadding(uint64_t rowSizeWithoutPadding) {
    return ((rowSizeWithoutPadding + 3) &~ 3);
}

template<typename T1, typename T2>
inline auto divideWithCeil(T1 a, T2 b) {
    return (a + b - 1) / b;
}

enum class WorkMode {
    INCREASE,
    DECREASE
};
