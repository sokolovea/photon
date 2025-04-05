#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <array>
#include <fstream>
#include <string>
#include <QString>
#include "bitmap.h"
#include "bitmap_util.h"

using namespace std;

OperationStatus readImageHeadersToStructure(const string& fileName, Bitmap24Image& bitmap24Image)
{
    ifstream sourceImageFile(fileName, std::ios::binary);

    if (!sourceImageFile.is_open()) {
        cerr << "Could not open file!" << endl;
        return OperationStatus::Failed;
    }

    sourceImageFile.read((char*) &(bitmap24Image.bitmapFileHeader), sizeof(BitmapFileHeader));
    if (bitmap24Image.bitmapFileHeader.bfType != 0x4d42) {
        cerr << "Not BMP file!" << endl;
        return OperationStatus::Failed;
    }

    sourceImageFile.read((char*) &bitmap24Image.bitmapInfoHeaderV3, sizeof(BitmapInfoHeaderV3));
    if (bitmap24Image.bitmapInfoHeaderV3.biBitCount != 24) {
        cerr << "Not 24 bit BMP file!" << endl;
        return OperationStatus::Failed;
    }

    return OperationStatus::Success;

}

OperationStatus readImagePixelsToBuffer(const string& fileName, uint8_t* outputBuffer, Bitmap24Image& bitmap24Image)
{
    int32_t inputWidthPx = bitmap24Image.bitmapInfoHeaderV3.biWidth;
    int32_t inputHeightPx = bitmap24Image.bitmapInfoHeaderV3.biHeight;
    uint64_t inputRowBytesCount = inputWidthPx * 3;
    uint64_t inputRowBytesCountWithPadding = getRowSizeWithPadding(inputRowBytesCount);
    uint64_t imageSize = inputRowBytesCountWithPadding * inputHeightPx;

    ifstream sourceImageFile(fileName, std::ios::binary);

    if (!sourceImageFile.is_open()) {
        cerr << "Could not open file!" << endl;
        return OperationStatus::Failed;
    }

    sourceImageFile.seekg(bitmap24Image.bitmapFileHeader.bfOffBits, ios_base::cur);
    // sourceImageFile.read((char*)(outputBuffer), imageSize);

    for (int32_t i = 0; i < inputHeightPx - 1; i++) {
        sourceImageFile.read((char*)(outputBuffer + i * inputRowBytesCountWithPadding), inputRowBytesCount);
        if (!sourceImageFile) {
            std::cerr << "Error reading data from file: " << std::endl;
            return OperationStatus::Failed;
        }
        sourceImageFile.seekg(inputRowBytesCountWithPadding - inputRowBytesCount, ios_base::cur);
    }

    return OperationStatus::Success;
}

OperationStatus writeImageStructureToFile(const Bitmap24Image& bitmap24Image, const uint8_t *inputBuffer, const std::string& fileName)
{
    std::ofstream targetImageFile(fileName, std::ios::binary);

    if (!targetImageFile.is_open()) {
        std::cerr << "Could not open file!" << std::endl;
        return OperationStatus::Failed;
    }

    if (bitmap24Image.bitmapFileHeader.bfType != 0x4d42) {
        std::cerr << "Not a BMP file!" << std::endl;
        return OperationStatus::Failed;
    }

    if (bitmap24Image.bitmapInfoHeaderV3.biBitCount != 24) {
        std::cerr << "Not a 24-bit BMP file!" << std::endl;
        return OperationStatus::Failed;
    }

    targetImageFile.write((char*)(&bitmap24Image.bitmapFileHeader), sizeof(BitmapFileHeader));
    if (!targetImageFile) {
        std::cerr << "Error writing Bitmap Headers to file!" << std::endl;
        return OperationStatus::Failed;
    }

    targetImageFile.write((char*)(&bitmap24Image.bitmapInfoHeaderV3), sizeof(BitmapInfoHeaderV3));
    if (!targetImageFile) {
        std::cerr << "Error writing BitmapInfoHeaderV3 to file!" << std::endl;
        return OperationStatus::Failed;
    }

    int32_t outputWidthPx = bitmap24Image.bitmapInfoHeaderV3.biWidth;
    int32_t outputHeightPx = bitmap24Image.bitmapInfoHeaderV3.biHeight;
    uint64_t outputRowBytesCount = outputWidthPx * 3;
    uint64_t padding = getRowSizeWithPadding(outputRowBytesCount) - outputRowBytesCount;

    // targetImageFile.write((char*)(inputBuffer), getRowSizeWithPadding(outputRowBytesCount) * outputHeightPx);

    char emptyBytes[] = { 0, 0, 0 };
    for (int32_t i = 0; i < bitmap24Image.bitmapInfoHeaderV3.biHeight; i++) {
        targetImageFile.write((char*)(inputBuffer + i * (outputRowBytesCount + padding)), outputRowBytesCount);
        targetImageFile.write(emptyBytes, padding);
        if (!targetImageFile) {
            std::cerr << "Error writing pixel data to file!" << std::endl;
            return OperationStatus::Failed;
        }
    }

    targetImageFile.close();
    return OperationStatus::Success;
}


