#pragma once

#include <cstdint>
#include <memory>
#include <cmath>
#include <stdexcept>

enum ImageChannel : uint8_t {
    Red    = 0b001,
    Green  = 0b010,
    Blue   = 0b100
};

#pragma pack(push, 1)
struct RGBPixel
{
    int64_t blue;
    int64_t green;
    int64_t red;

    static uint64_t getMaxChannelValue() {
        return 255;
    }
};
#pragma pack(pop)

template <typename T>
class   ImageRowsRingBuffer
{
private:
    std::unique_ptr<T[]> data;
    uint64_t width;
    uint64_t widthBytes;
    uint64_t length;
    uint64_t beginIndex;
    std::unique_ptr<T[]> resultRow;

    std::unique_ptr<RGBPixel[]> sumColsBuffer;


    static const uint64_t mirrorDimention(int64_t value, int64_t rightBorderNotInclusive) {
        if (value < 0) {
            value = std::abs(value);
        }
        if (value >= rightBorderNotInclusive) {
            value %= (2 * rightBorderNotInclusive);
            value = 2 * rightBorderNotInclusive - value - 1;
        }
        return value;
    }

    const uint64_t mirrorColIndex(int64_t value) const{
        return mirrorDimention(value, width);
    }

    static uint8_t getNormalizedChannelValue(int64_t value, int64_t maxChannelValue) {
        if (value < 0) {
            value = std::abs(value);
        }
        if (value > maxChannelValue) {
            value = maxChannelValue;
        }
        return value;
    }


public:
    ImageRowsRingBuffer(uint64_t rows, uint64_t cols, uint64_t padding = 0) {
        width = cols;
        widthBytes = cols * sizeof(T);
        length = rows;
        beginIndex = 0;
        data = std::make_unique<T[]>(rows * widthBytes);
        resultRow = std::make_unique<T[]>(widthBytes + padding);
        sumColsBuffer = std::make_unique<RGBPixel[]>(cols);
        for (int64_t i = 0; i < cols; i++) {
            sumColsBuffer[i].red = 0;
            sumColsBuffer[i].green = 0;
            sumColsBuffer[i].blue = 0;
        }
    }

    const T* pushNewRowAndGetPtr() {
        uint32_t oldBeginIndex = beginIndex;
        beginIndex = (beginIndex + 1) % length;
        return data.get() + oldBeginIndex * widthBytes;
    }

    T* getRow(uint64_t row) {
        uint64_t realRow = (beginIndex + row) % length;
        return data.get() + realRow * widthBytes;
    }

    const T* getRow(uint64_t row) const {
        uint64_t realRow = (beginIndex + row) % length;
        return data.get() + realRow * widthBytes;
    }

    const T getElem(uint64_t row, int64_t col) const {
        return getRow(row)[mirrorColIndex(col)];
    }

    T* applyVerticalKernel(uint64_t kernelHeight) const {
        if (kernelHeight != length) {
            throw std::invalid_argument("Kernel with this size cannot be applied to buffer!");
        }

        int64_t rowCenterOffset = (kernelHeight - 1) / 2;
        int64_t dividor = kernelHeight;

        for (uint64_t k = 0; k < width; k++) {
            int64_t red = 0;
            int64_t green = 0;
            int64_t blue = 0;

            auto currentPixel = sumColsBuffer[k];

            red = currentPixel.red;
            green = currentPixel.green;
            blue = currentPixel.blue;

            auto centerPixel = getElem(rowCenterOffset, k);

            resultRow[k].red = getNormalizedChannelValue(red / dividor, T::getMaxChannelValue());
            resultRow[k].green = getNormalizedChannelValue(green / dividor, T::getMaxChannelValue());
            resultRow[k].blue = getNormalizedChannelValue(blue / dividor, T::getMaxChannelValue());
        }

        return resultRow.get();
    }


    void applyHorizontalKernelToLastRow(uint64_t kernelWidth) {
        int64_t colCenterOffset = (kernelWidth - 1) / 2;
        int64_t dividor = kernelWidth;

        int64_t red = 0;
        int64_t green = 0;
        int64_t blue = 0;
        for (int64_t j = 0; j < kernelWidth; j++) {
            auto currentPixel = getElem(length - 1, j - kernelWidth / 2);
            red += currentPixel.red;
            green += currentPixel.green;
            blue += currentPixel.blue;
        }


        for (uint64_t k = 0; k < width; k++) {
            auto centerPixel = getElem(length - 1, k);
            resultRow[k].red = getNormalizedChannelValue(red / dividor, T::getMaxChannelValue());
            resultRow[k].green = getNormalizedChannelValue(green / dividor, T::getMaxChannelValue());
            resultRow[k].blue = getNormalizedChannelValue(blue / dividor, T::getMaxChannelValue());

            auto delPixel = getElem(length - 1, k - kernelWidth / 2);
            auto addPixel = getElem(length - 1, k + (kernelWidth + 1) / 2);

            red -= delPixel.red;
            red += addPixel.red;

            green -= delPixel.green;
            green += addPixel.green;

            blue -= delPixel.blue;
            blue += addPixel.blue;
        }

        memcpy(getRow(length - 1), resultRow.get(), widthBytes);
    }

    void updateSumColsBufferByRow(uint64_t row, int8_t sign) {
        if (sign < 0) {
            for (int64_t i = 0; i < width; i++) {
                auto currentPixel = getRow(row)[i];
                sumColsBuffer[i].red -= currentPixel.red;
                sumColsBuffer[i].green -= currentPixel.green;
                sumColsBuffer[i].blue -= currentPixel.blue;
            }
        } else {
            for (int64_t i = 0; i < width; i++) {
                auto currentPixel = getRow(row)[i];
                sumColsBuffer[i].red += currentPixel.red;
                sumColsBuffer[i].green += currentPixel.green;
                sumColsBuffer[i].blue += currentPixel.blue;
            }
        }

    }

    void updateFullColsBuffer() {
        for (int64_t i = 0; i < width; i++) {
            sumColsBuffer[i].red = 0;
            sumColsBuffer[i].green = 0;
            sumColsBuffer[i].blue = 0;
        }
        for (uint64_t j = 0; j < length; j++) {
            for (int64_t i = 0; i < width; i++) {
                auto currentPixel = getRow(j)[i];
                sumColsBuffer[i].red += currentPixel.red;
                sumColsBuffer[i].green += currentPixel.green;
                sumColsBuffer[i].blue += currentPixel.blue;
            }
        }
    }
};
