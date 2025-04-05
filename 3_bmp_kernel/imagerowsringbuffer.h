#pragma once

#include <cstdint>
#include <memory>
#include <cmath>
#include "kernel.h"

enum ImageChannel : uint8_t {
    Red    = 0b001,
    Green  = 0b010,
    Blue   = 0b100
};

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


    const uint64_t mirrorColIndex(int64_t value) const {
        return mirrorDimention(value, width);
    }

    uint8_t getNormalizedChannelValue(int64_t value, int64_t maxChannelValue) const {
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
    }

    const T* pushNewRowAndGetPtr() {
        uint32_t oldBeginIndex = beginIndex;
        beginIndex = (beginIndex + 1) % length;
        return data.get() + oldBeginIndex * widthBytes;
    }

    const T* getRow(uint64_t row) const {
        uint64_t realRow = (beginIndex + row) % length;
        return data.get() + realRow * widthBytes;
    }

    const T getElem(uint64_t row, int64_t col) const {
        return getRow(row)[mirrorColIndex(col)];
    }

    const uint64_t mirrorDimention(int64_t value, int64_t rightBorderNotInclusive) const {
        if (value < 0) {
            value = std::abs(value);
        }
        if (value >= rightBorderNotInclusive) {
            value %= (2 * rightBorderNotInclusive);
            value = 2 * rightBorderNotInclusive - value - 1;
        }
        return value;
    }

    T* applyKernel(const Kernel& kernel, uint8_t channelMask) const {
        if (kernel.getHeight() != length || kernel.getWidth() > widthBytes) {
            throw std::invalid_argument("Kernel with this size cannot be applied to buffer!");
        }

        int64_t rowCenterOffset = (kernel.getHeight() - 1) / 2;
        int64_t colCenterOffset = (kernel.getWidth() - 1) / 2;

        for (uint64_t k = 0; k < width; k++) {
            double red = 0;
            double green = 0;
            double blue = 0;

            for (int64_t i = 0; i < kernel.getHeight(); i++) {
                for (int64_t j = 0; j < kernel.getWidth(); j++) {
                    auto currentPixel = getElem(i, k + j);

                    if (channelMask & Red) {
                        red += kernel[i][j] * currentPixel.red;
                    }
                    if (channelMask & Green) {
                        green += kernel[i][j] * currentPixel.green;
                    }
                    if (channelMask & Blue) {
                        blue += kernel[i][j] * currentPixel.blue;
                    }
                }
            }

            auto centerPixel = getElem(rowCenterOffset, k);

            resultRow[k].red = (channelMask & Red) ? getNormalizedChannelValue(red, T::getMaxChannelValue()) : centerPixel.red;
            resultRow[k].green = (channelMask & Green) ? getNormalizedChannelValue(green, T::getMaxChannelValue()) : centerPixel.green;
            resultRow[k].blue = (channelMask & Blue) ? getNormalizedChannelValue(blue, T::getMaxChannelValue()) : centerPixel.blue;
        }

        return resultRow.get();
    }
};
