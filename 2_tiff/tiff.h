#pragma once
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <fstream>
#include <array>
#include <cmath>
#include <numeric>

#define COLORS_NUM UINT16_MAX + 1

struct Tiff16RGBPixel
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct TiffFileHeader
{
    uint16_t byteOrder;
    uint16_t versionNumber;
    uint32_t offsetIFD;
};

struct TiffIFDEntry {
    uint16_t tag;
    uint16_t fieldType;
    uint32_t numberValues;
    uint32_t valueOffset;
};


struct TiffIFD {
    uint16_t numberEntries;
    std::unique_ptr<TiffIFDEntry[]> entries;
    uint32_t padding;
    TiffIFD(uint16_t numberEntries): padding(0) {
        this->numberEntries = numberEntries;
        entries = std::make_unique<TiffIFDEntry[]>(numberEntries);
    }
};

enum TiffTagEnum : uint16_t {
    ImageWidth = 256,
    ImageLength = 257,
    BitsPerSample = 258,
    Compression = 259,
    PhotometricInterpretation = 262,
    StripOffsets = 273,
    SamplesPerPixel = 277,
    RowsPerStrip = 278,
    StripByteCounts = 279,
    PlanarConfiguration = 284
};

enum class FieldType : uint16_t {
    Byte = 1,
    Ascii = 2,
    Short = 3,
    Long = 4,
    Rational = 5,
    /* In Tiff 6.0 additionally */
    Sbyte = 6,
    Undefined = 7,
    Sshort = 8,
    Slong = 9,
    Srational = 10,
    Float = 11,
    Double = 12
};

struct ImageChannelStatistics {
    uint16_t max;
    uint16_t min;
    uint16_t mean;
    uint16_t leftBorder;
    uint16_t rightBorder;
    friend std::ostream& operator<<(std::ostream& os, const ImageChannelStatistics& obj);
};

std::ostream& operator<<(std::ostream& os, const ImageChannelStatistics& obj) {
    os << "min = " << obj.min << "; max = " << obj.max << "; mean = " << obj.mean;
    return os;
}

struct Tiff16RGBImageStatistics {
    ImageChannelStatistics red;
    ImageChannelStatistics green;
    ImageChannelStatistics blue;
    friend std::ostream& operator<<(std::ostream& os, const Tiff16RGBImageStatistics& obj);
};

std::ostream& operator<<(std::ostream& os, const Tiff16RGBImageStatistics& obj) {
    os << "Red sample: " << obj.red << std::endl;
    os << "Green sample: " << obj.green << std::endl;
    os << "Blue sample: " << obj.blue << std::endl;
    return os;
}

struct Tiff16RGBImage
{
    std::unique_ptr<TiffFileHeader> tiffFileHeader;
    std::unique_ptr<TiffIFD> tiffIFD;
    std::ifstream inputStream;
    std::ofstream outputStream;

    uint32_t width;
    uint32_t height;
    std::vector<uint64_t> bitsPerSample;
    std::vector<uint64_t> stripOffsets;
    uint16_t compression;
    uint16_t rowsPerStrip;

    Tiff16RGBImage() {
    };

    static uint64_t getFieldSize(FieldType fieldType) {
        switch(fieldType) {
        case FieldType::Byte:
        case FieldType::Sbyte:
        case FieldType::Undefined:
            return 1;
        case FieldType::Ascii:
            return 1;
        case FieldType::Short:
        case FieldType::Sshort:
            return 2;
        case FieldType::Long:
        case FieldType::Slong:
        case FieldType::Float:
            return 4;
        case FieldType::Rational:
        case FieldType::Srational:
        case FieldType::Double:
            return 8;
        default:
            return 1;
        }
    }

    void setValueByKey(uint16_t key, uint64_t valueOffset, uint64_t numberValues = 1) {
        if (key == TiffTagEnum::ImageWidth) {
            width = valueOffset;
        } else if (key == TiffTagEnum::ImageLength) {
            height = valueOffset;
        } else if (key == TiffTagEnum::BitsPerSample) {
            bitsPerSample = getValuesVector(key, valueOffset, numberValues, FieldType::Short);
        } else if (key == TiffTagEnum::Compression) {
            compression = valueOffset;
        } else if (key == TiffTagEnum::StripOffsets) {
            stripOffsets = getValuesVector(key, valueOffset, numberValues, FieldType::Long);
        } else if (key == TiffTagEnum::RowsPerStrip) {
            rowsPerStrip = valueOffset;
        }
    }

    std::vector<uint64_t> getValuesVector(uint16_t key, uint64_t valueOffset, uint64_t numberValues, FieldType fieldType) {
        std::streampos oldPos = inputStream.tellg();
        inputStream.seekg(valueOffset, std::ios::beg);
        std::vector<uint64_t> values;
        for (uint32_t j = 0; j < numberValues; j++) {
            uint64_t sample = 0;
            uint32_t size = getFieldSize(fieldType);
            inputStream.read(reinterpret_cast<char*>(&sample), static_cast<uint32_t>(size));
            if (inputStream.fail()) {
                std::cerr << "Error reading values by key = " << key << "!" << std::endl;
                throw std::runtime_error("Error reading tag!");
            }
            values.push_back(sample);
        }
        inputStream.seekg(oldPos, std::ios::beg);
        return values;
    }


    std::pair<uint16_t, uint16_t> getBorders(const std::array<uint64_t, COLORS_NUM>& histogram) {
        bool isMinFound = false;
        uint16_t minIndex = 0;
        uint16_t maxIndex = 0;
        for (size_t i = 0; i < histogram.size(); i++) {
            if (histogram[i] != 0) {
                if (!isMinFound) {
                    isMinFound = true;
                    minIndex = i;
                }
                maxIndex = i;
            }
        }
        return std::pair<uint16_t, uint16_t>(minIndex, maxIndex);
    }

    uint64_t calculateMean(const std::array<uint64_t, COLORS_NUM>& histogram) const {
        double sum = 0;
        uint64_t totalPixels = width * height;

        for (size_t i = 0; i < histogram.size(); i++) {
            uint16_t brightness = i;
            uint64_t count = histogram[i];
            sum += brightness * count;
        }
        if (totalPixels == 0) {
            return 0;
        }
        return round(sum / totalPixels);
    }


    std::pair<uint16_t, uint16_t> calculateRealBorders(const std::array<uint64_t, COLORS_NUM>& histogram, uint16_t leftBorder, uint16_t rightBorder,
                                                       double discardedPixelFractionLeft, double discardedPixelFractionRight)
    {
        uint64_t totalPixels = width * height;
        uint64_t pixelsToDiscardLeft = totalPixels * discardedPixelFractionLeft;
        uint64_t leftSum = 0;
        uint16_t newLeftBorder = leftBorder;
        for (int32_t i = leftBorder; i <= rightBorder; i++) {
            leftSum += histogram[i];
            if (leftSum >= pixelsToDiscardLeft) {
                newLeftBorder = i;
                break;
            }
        }

        uint64_t pixelsToDiscardRight = totalPixels * discardedPixelFractionRight;
        uint64_t rightSum = 0;
        uint16_t newRightBorder = rightBorder;
        for (int32_t i = rightBorder; i >= leftBorder; i--) {
            rightSum += histogram[i];
            if (rightSum >= pixelsToDiscardRight) {
                newRightBorder = i;
                break;
            }
        }

        return {newLeftBorder, newRightBorder};
    }

    Tiff16RGBImageStatistics getStatistics(double discardedPixelFractionCoeffLeft, double discardedPixelFractionCoeffRight) {
        Tiff16RGBImageStatistics tiff16RGBImageStatistics;
        std::array<uint64_t, COLORS_NUM> redChannelPixelMap = {0};
        std::array<uint64_t, COLORS_NUM> greenChannelPixelMap = {0};
        std::array<uint64_t, COLORS_NUM> blueChannelPixelMap = {0};

        std::streampos oldPos = inputStream.tellg();
        uint32_t widthPx = width;
        uint64_t rowBytesCount = widthPx * sizeof(Tiff16RGBPixel);
        uint32_t heightPx = height;
        std::unique_ptr<Tiff16RGBPixel[]> inputRow = std::make_unique<Tiff16RGBPixel[]>(widthPx);
        inputStream.seekg(stripOffsets[0], std::ios_base::beg);
        for (uint64_t i = 0; i < heightPx; i++) {
            inputStream.read((char*)inputRow.get(), rowBytesCount);
            if (inputStream.fail()) {
                std::cerr << "Error reading source image!" << std::endl;
                throw std::runtime_error("Error reading source image!");
            }
            for (uint64_t j = 0; j < widthPx; j++) {
                Tiff16RGBPixel tiff16RGBPixel = inputRow.get()[j];
                redChannelPixelMap[tiff16RGBPixel.red]++;
                greenChannelPixelMap[tiff16RGBPixel.green]++;
                blueChannelPixelMap[tiff16RGBPixel.blue]++;
            }
        }
        //std::cout << "redChannelPixelMap sum = " << std::reduce(redChannelPixelMap.begin(), redChannelPixelMap.end()) << std::endl;
        inputStream.seekg(oldPos, std::ios::beg);
        auto minMaxIndex = getBorders(redChannelPixelMap);

        tiff16RGBImageStatistics.red.min = minMaxIndex.first;
        tiff16RGBImageStatistics.red.max = minMaxIndex.second;
        tiff16RGBImageStatistics.red.mean = calculateMean(redChannelPixelMap);

        auto realBorders = calculateRealBorders(redChannelPixelMap, tiff16RGBImageStatistics.red.min, tiff16RGBImageStatistics.red.max,
                                                discardedPixelFractionCoeffLeft, discardedPixelFractionCoeffRight);
        tiff16RGBImageStatistics.red.leftBorder = realBorders.first;
        tiff16RGBImageStatistics.red.rightBorder = realBorders.second;

        minMaxIndex = getBorders(greenChannelPixelMap);
        tiff16RGBImageStatistics.green.min = minMaxIndex.first;
        tiff16RGBImageStatistics.green.max = minMaxIndex.second;
        tiff16RGBImageStatistics.green.mean = calculateMean(greenChannelPixelMap);

        realBorders = calculateRealBorders(greenChannelPixelMap, tiff16RGBImageStatistics.green.min, tiff16RGBImageStatistics.green.max,
                                                discardedPixelFractionCoeffLeft, discardedPixelFractionCoeffRight);
        tiff16RGBImageStatistics.green.leftBorder = realBorders.first;
        tiff16RGBImageStatistics.green.rightBorder = realBorders.second;

        minMaxIndex = getBorders(blueChannelPixelMap);
        tiff16RGBImageStatistics.blue.min = minMaxIndex.first;
        tiff16RGBImageStatistics.blue.max = minMaxIndex.second;
        tiff16RGBImageStatistics.blue.mean = calculateMean(blueChannelPixelMap);

        realBorders = calculateRealBorders(blueChannelPixelMap, tiff16RGBImageStatistics.blue.min, tiff16RGBImageStatistics.blue.max,
                                           discardedPixelFractionCoeffLeft, discardedPixelFractionCoeffRight);
        tiff16RGBImageStatistics.blue.leftBorder = realBorders.first;
        tiff16RGBImageStatistics.blue.rightBorder = realBorders.second;


        return tiff16RGBImageStatistics;
    }

    void checkAndPrintNecessaryInformation() {
        if (compression != 1) {
            throw std::runtime_error("compression != 1");
        }
        if (bitsPerSample.size() != 3) {
            throw std::runtime_error("samples != 3");
        }
        if (bitsPerSample[0] != 16) {
            throw std::runtime_error("bits per sample != 16");
        }

        std::cout << "Necessary image parameters:" << std::endl;
        std::cout << "Width: " << width << std::endl;
        std::cout << "Height: " << height << std::endl;
        std::cout << "Bits per sample: " << bitsPerSample.size() << " * " << bitsPerSample[0]<< std::endl;
        std::cout << "Compression: " << compression << std::endl;
        std::cout << "Rows per strip: " << rowsPerStrip << std::endl;
    }
};


