#pragma once

#include <cstdint>
#include <memory>
#include <cmath>
#include <stdexcept>
#include "pixel.h"

template <typename T, typename PixelStatistics>
class   ImageRowsRingBuffer
{
private:
    std::unique_ptr<T[]> data;
    int64_t width;
    int64_t widthBytes;
    int64_t length;
    int64_t beginIndex;
    std::unique_ptr<T[]> resultRow;

    std::unique_ptr<PixelStatistics[]> sumColsBuffer;


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
    ImageRowsRingBuffer(uint64_t rows, int64_t cols, uint64_t padding = 0) {
        width = cols;
        widthBytes = cols * sizeof(T);
        length = rows;
        beginIndex = 0;
        data = std::make_unique<T[]>(rows * widthBytes);
        resultRow = std::make_unique<T[]>(widthBytes + padding);
        sumColsBuffer = std::make_unique<PixelStatistics[]>(cols);
        for (int64_t i = 0; i < cols; i++) {
            PixelStatisticsTraits<PixelStatistics>::reset(sumColsBuffer[i]);
        }
    }

    T* pushNewRowAndGetPtr() {
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

    T* applyVerticalKernel(int64_t kernelHeight) const {
        if (kernelHeight != length) {
            throw std::invalid_argument("Kernel with this size cannot be applied to buffer!");
        }

        int64_t dividor = kernelHeight;

        for (int64_t k = 0; k < width; k++) {

            auto currentPixel = sumColsBuffer[k];
            resultRow[k] = PixelStatisticsTraits<PixelStatistics>::normalize(currentPixel, dividor);
        }

        return resultRow.get();
    }


    void applyHorizontalKernelToLastRow(int64_t kernelWidth) {
        int64_t dividor = kernelWidth;

        PixelStatistics pixelStatistics;
        for (int64_t j = 0; j < kernelWidth; j++) {
            auto currentPixel = getElem(length - 1, j - kernelWidth / 2);
            PixelStatisticsTraits<PixelStatistics>::add(pixelStatistics, currentPixel);
        }


        for (int64_t k = 0; k < width; k++) {
            resultRow[k] = PixelStatisticsTraits<PixelStatistics>::normalize(pixelStatistics, dividor);

            auto delPixel = getElem(length - 1, k - kernelWidth / 2);
            auto addPixel = getElem(length - 1, k + (kernelWidth + 1) / 2);

            PixelStatisticsTraits<PixelStatistics>::subtract(pixelStatistics, delPixel);
            PixelStatisticsTraits<PixelStatistics>::add(pixelStatistics, addPixel);

        }

        memcpy(getRow(length - 1), resultRow.get(), widthBytes);
    }

    void updateSumColsBufferByRow(int64_t row, int8_t sign) {
        if (sign < 0) {
            for (int64_t i = 0; i < width; i++) {
                auto currentPixel = getRow(row)[i];
                PixelStatisticsTraits<PixelStatistics>::subtract(sumColsBuffer[i], currentPixel);
            }
        } else {
            for (int64_t i = 0; i < width; i++) {
                auto currentPixel = getRow(row)[i];
                PixelStatisticsTraits<PixelStatistics>::add(sumColsBuffer[i], currentPixel);
            }
        }

    }

    void updateFullColsBuffer() {
        for (int64_t i = 0; i < width; i++) {
            PixelStatisticsTraits<PixelStatistics>::reset(sumColsBuffer[i]);
        }
        for (int64_t j = 0; j < length; j++) {
            for (int64_t i = 0; i < width; i++) {
                auto currentPixel = getRow(j)[i];
                PixelStatisticsTraits<PixelStatistics>::add(sumColsBuffer[i], currentPixel);
            }
        }
    }
};
