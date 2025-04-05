#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <memory>
#include <utility>
#include "bitmap.h"
#include "bitmap_util.h"

using namespace std;

enum class WorkMode {
    INCREASE,
    DECREASE
};

int main(int argc, char** argv) {
    if (argc != 5) {
        cerr << "Wrong parameters count!" << endl;
        return 1;
    }

    ifstream inputStream;
    inputStream.open(argv[3], std::ios_base::binary);

    ofstream outputStream;
    outputStream.open(argv[4], std::ios_base::binary);

    if (!inputStream.is_open() || !outputStream.is_open()) {
        cerr << "Can't open files!" << endl;
        return 2;
    }

    BitmapFileHeader bitmapInputFileHeader;
    BitmapInfoHeader bitmapInputInfoHeader;

    inputStream.read((char*)&bitmapInputFileHeader, sizeof(BitmapFileHeader));
    if (bitmapInputFileHeader.bfType != 0x4d42) {
        cerr << "Not BMP file!" << endl;
        return 3;
    }

    inputStream.read((char*)&bitmapInputInfoHeader, sizeof(BitmapInfoHeader));

    if (bitmapInputInfoHeader.biBitCount != 24) {
        cerr << "Not 24 bit BMP file!" << endl;
        return 4;
    }

    WorkMode workMode;
    if (!strcmp("-inc", argv[1])) {
        workMode = WorkMode::INCREASE;
    }
    else if (!strcmp("-dec", argv[1])) {
        workMode = WorkMode::DECREASE;
    } else {
        cerr << "Unknown work mode \"" << argv[1] << "\"!" << endl;
        return 5;
    }

    int32_t coeff = std::atoi(argv[2]);
    if (coeff <= 0) {
        cerr << "Coefficient is not correct!" << endl;
        return 6;
    }

    int32_t inputWidthPx = bitmapInputInfoHeader.biWidth;
    int32_t inputHeightPx = bitmapInputInfoHeader.biHeight;

    int32_t outputWidthPx = workMode == WorkMode::INCREASE? inputWidthPx * coeff : divideWithCeil(inputWidthPx, coeff);
    int32_t outputHeight = workMode == WorkMode::INCREASE? inputHeightPx * coeff : divideWithCeil(inputHeightPx, coeff);

    // if (outputWidth <= 0 || inputWidth <= 0) {
    //     cerr << "Coefficient is too large!" << endl;
    //     return 5;
    // }

    BitmapFileHeader bitmapOutputFileHeader = bitmapInputFileHeader;
    BitmapInfoHeader bitmapOutputInfoHeader = bitmapInputInfoHeader;

    uint64_t inputRowBytesCount = inputWidthPx * 3;
    uint64_t outputRowBytesCount = outputWidthPx * 3;

    uint64_t inputRowBytesCountWithPadding = getRowSizeWithPadding(inputRowBytesCount);
    uint64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCount);

    std::unique_ptr<uint8_t[]> inputRow = std::make_unique<uint8_t[]>(inputRowBytesCountWithPadding);
    std::unique_ptr<uint8_t[]> outputRow = std::make_unique<uint8_t[]>(outputRowBytesCountWithPadding);

    bitmapOutputFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + outputRowBytesCount * outputHeight;
    bitmapOutputFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
    outputStream.write((char*)&bitmapOutputFileHeader, sizeof(BitmapFileHeader));

    bitmapOutputInfoHeader.biWidth = outputWidthPx;
    bitmapOutputInfoHeader.biHeight = outputHeight;
    outputStream.write((char*)&bitmapOutputInfoHeader, sizeof(BitmapInfoHeader));

    if (workMode == WorkMode::INCREASE) {
        for (int32_t i = 0; i < inputHeightPx; i++) {
            inputStream.read((char*)inputRow.get(), inputRowBytesCountWithPadding);
            if (inputStream.fail()) {
                cerr << "Error reading source image!" << endl;
                return 6;
            }
            increaseResolution((Bitmap24Pixel*)inputRow.get(), (Bitmap24Pixel*)outputRow.get(), inputWidthPx, outputWidthPx, coeff);
            for (int32_t duplication = 0; duplication < coeff; duplication++) {
                outputStream.write((char*)outputRow.get(), outputRowBytesCountWithPadding);
                if (outputStream.fail()) {
                    cerr << "Error writing to file!" << endl;
                    return 7;
                }
            }
        }
    } else if (workMode == WorkMode::DECREASE) {
        uint32_t iterationsCount = divideWithCeil(inputHeightPx, coeff);
        for (uint32_t i = 0; i < iterationsCount; i++) {
            inputStream.read((char*)inputRow.get(), inputRowBytesCountWithPadding);
            if (inputStream.fail()) {
                cerr << "Error reading source image!" << endl;
                return 6;
            }
            inputStream.seekg(inputRowBytesCountWithPadding * (coeff - 1), ios_base::cur);
            decreaseResolution((Bitmap24Pixel*)inputRow.get(), (Bitmap24Pixel*)outputRow.get(), inputWidthPx, outputWidthPx, coeff);
            outputStream.write((char*)outputRow.get(), outputRowBytesCountWithPadding);
            if (outputStream.fail()) {
                cerr << "Error writing to file!" << endl;
                return 7;
            }
        }
    }
    outputStream.close();
    inputStream.close();
    return 0;
}