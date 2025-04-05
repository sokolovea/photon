#include "bitmap.h"
#include "bitmapmatrix.h"

#include <cstring>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <memory>

using namespace std;


int main(int argc, char** argv) {
    if (argc < 6) {
        cerr << "Wrong parameters count!" << endl;
        return 1;
    }

    ifstream inputStream;
    inputStream.open(argv[1], ios_base::binary);

    ofstream outputStream;
    outputStream.open(argv[2], ios_base::binary);

    if (!inputStream.is_open() || !outputStream.is_open()) {
        cerr << "Can't open input files!" << endl;
        return 2;
    }

    double degrees = 0;
    try {
        degrees = stod(argv[3]);
    } catch (const std::exception& e) {
        cerr << "Can't parse degrees!" << endl;
        return 3;
    }

    InterpolationMode interpolationMode;
    if (!strcmp("--nearestNeighbour", argv[4])) {
        interpolationMode = InterpolationMode::NearestNeighbour;
    } else if (!strcmp("--bilinear", argv[4])) {
        interpolationMode = InterpolationMode::Bilinear;
    } else if (!strcmp("--bicubic", argv[4])) {
        interpolationMode = InterpolationMode::Bicubic;
    } else if (!strcmp("--lanczos3", argv[4])) {
        interpolationMode = InterpolationMode::Lanczos3;
    } else {
        cerr << "Can't parse interpolation mode!" << endl;
        return 4;
    }

    double zoom = 0;
    try {
        zoom = stod(argv[5]);
    } catch (const std::exception& e) {
        cerr << "Can't parse zoom!" << endl;
        return 5;
    }
    if (zoom <= 0) {
        cerr << "Zoom should be > 0!" << endl;
        return 5;
    }

    BitmapFileHeader bitmapInputFileHeader;
    BitmapInfoHeaderV3 bitmapInputInfoHeader;

    inputStream.read((char*)&bitmapInputFileHeader, sizeof(BitmapFileHeader));
    if (bitmapInputFileHeader.bfType != 0x4d42 || inputStream.fail()) {
        cerr << "Not BMP file!" << endl;
        return 5;
    }

    inputStream.read((char*)&bitmapInputInfoHeader, sizeof(BitmapInfoHeaderV3));
    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);

    if (inputStream.fail()) {
        cerr << "Error reading source file!" << endl;
        return 6;
    }

    if (bitmapInputInfoHeader.biBitCount != 24) {
        cerr << "Not 24 bit BMP file!" << endl;
        return 7;
    }

    int32_t inputWidthPx = bitmapInputInfoHeader.biWidth;
    int32_t inputHeightPx = bitmapInputInfoHeader.biHeight;

    BitmapFileHeader bitmapOutputFileHeader = bitmapInputFileHeader;
    BitmapInfoHeaderV3 bitmapOutputInfoHeader = bitmapInputInfoHeader;

    uint64_t inputRowBytesCountWithoutPadding = inputWidthPx * 3;

    uint64_t inputRowBytesCountWithPadding = getRowSizeWithPadding(inputRowBytesCountWithoutPadding);

    uint64_t inputPaddingBytes = inputRowBytesCountWithPadding - inputRowBytesCountWithoutPadding;

    BitmapMatrix inputBitmapMatrix(inputWidthPx, inputHeightPx);

    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);
    for (int64_t i = 0; i < inputHeightPx; i++) {
        uint64_t row = inputHeightPx - i - 1;
        inputStream.read(reinterpret_cast<char*>(inputBitmapMatrix(row)), inputRowBytesCountWithoutPadding);
        inputStream.seekg(inputPaddingBytes, ios::cur);
    }

    if (inputStream.fail()) {
        cerr << "Error reading source file!" << endl;
        return 8;
    }

    ImageNecessaryInfo outputImageInfo = inputBitmapMatrix.getRotatedImageInfo(degrees * M_PI / 180, zoom);

    int64_t outputWidthPx = outputImageInfo.getWidth();
    int64_t outputHeightPx = outputImageInfo.getHeight();

    bitmapOutputInfoHeader.biWidth = outputWidthPx;
    bitmapOutputInfoHeader.biHeight = outputHeightPx;

    uint64_t outputRowBytesCountWithoutPadding = outputWidthPx * 3;
    uint64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCountWithoutPadding);
    uint64_t outputPaddingBytes = outputRowBytesCountWithPadding - outputRowBytesCountWithoutPadding;

    bitmapOutputFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + outputRowBytesCountWithPadding * outputHeightPx;
    bitmapOutputFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);
    outputStream.write((char*)&bitmapOutputFileHeader, sizeof(BitmapFileHeader));
    outputStream.write((char*)&bitmapOutputInfoHeader, sizeof(BitmapInfoHeaderV3));

    std::unique_ptr<Bitmap24Pixel[]> outputRow = std::make_unique<Bitmap24Pixel[]>(outputWidthPx);

    for (int64_t i = 0; i < outputHeightPx; i++) {
        uint64_t row = outputHeightPx - i - 1;
        inputBitmapMatrix.calculateOutputRow(row, outputImageInfo, outputRow.get(), interpolationMode);
        outputStream.write(reinterpret_cast<char*>(outputRow.get()), outputRowBytesCountWithoutPadding);
        outputStream.write(reinterpret_cast<char*>(outputRow.get()), outputPaddingBytes);
    }

    return 0;
}
