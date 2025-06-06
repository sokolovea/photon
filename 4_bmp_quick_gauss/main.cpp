#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <memory>
#include "bitmap.h"
#include "kernel.h"
#include "imagerowsringbuffer.h"

using namespace std;

void chooseKernel(Kernel& kernelVertical, Kernel& kernelHorizontal, uint8_t& channelMask) {

    uint64_t rows;
    uint64_t cols;
    double stdev;

    channelMask = ImageChannel::Red | ImageChannel::Green | ImageChannel::Blue;

    cout << "Using Gaussian filter!" << endl;
    cout << "Input rows and cols count: " << endl;
    cin >> rows >> cols;
    cout << "Input stdev: " << endl;
    cin >> stdev;
    kernelVertical.setHeight(rows);
    kernelHorizontal.setWidth(cols);

    kernelVertical.setToGaussianKernel(stdev);
    kernelVertical.setWidth(1);
    kernelHorizontal.setToGaussianKernel(stdev);
    kernelHorizontal.setHeight(1);
}


int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Wrong parameters count!" << endl;
        return 1;
    }

    ifstream inputStream;
    inputStream.open(argv[1], std::ios_base::binary);

    ofstream outputStream;
    outputStream.open(argv[2], std::ios_base::binary);

    if (!inputStream.is_open() || !outputStream.is_open()) {
        cerr << "Can't open files!" << endl;
        return 2;
    }

    BitmapFileHeader bitmapInputFileHeader;
    BitmapInfoHeaderV3 bitmapInputInfoHeader;

    inputStream.read((char*)&bitmapInputFileHeader, sizeof(BitmapFileHeader));
    if (bitmapInputFileHeader.bfType != 0x4d42 || inputStream.fail()) {
        cerr << "Not BMP file!" << endl;
        return 3;
    }

    inputStream.read((char*)&bitmapInputInfoHeader, sizeof(BitmapInfoHeaderV3));
    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);

    if (inputStream.fail()) {
        cerr << "Error reading source file!" << endl;
        return 4;
    }

    if (bitmapInputInfoHeader.biBitCount != 24) {
        cerr << "Not 24 bit BMP file!" << endl;
        return 5;
    }

    int32_t inputWidthPx = bitmapInputInfoHeader.biWidth;
    int32_t inputHeightPx = bitmapInputInfoHeader.biHeight;

    int32_t outputWidthPx = inputWidthPx;
    int32_t outputHeight = inputHeightPx;

    BitmapFileHeader bitmapOutputFileHeader = bitmapInputFileHeader;
    BitmapInfoHeaderV3 bitmapOutputInfoHeader = bitmapInputInfoHeader;

    uint64_t inputRowBytesCountWithoutPadding = inputWidthPx * 3;
    uint64_t outputRowBytesCountWithoutPadding = outputWidthPx * 3;

    uint64_t inputRowBytesCountWithPadding = getRowSizeWithPadding(inputRowBytesCountWithoutPadding);
    uint64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCountWithoutPadding);

    uint64_t paddingBytes = inputRowBytesCountWithPadding - inputRowBytesCountWithoutPadding;

    std::unique_ptr<uint8_t[]> inputRow = std::make_unique<uint8_t[]>(inputRowBytesCountWithPadding);
    std::unique_ptr<uint8_t[]> outputRow = std::make_unique<uint8_t[]>(outputRowBytesCountWithPadding);

    bitmapOutputFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + outputRowBytesCountWithPadding * outputHeight;
    bitmapOutputFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);
    outputStream.write((char*)&bitmapOutputFileHeader, sizeof(BitmapFileHeader));

    bitmapOutputInfoHeader.biWidth = outputWidthPx;
    bitmapOutputInfoHeader.biHeight = outputHeight;

    Kernel kernelVertical;
    Kernel kernelHorizontal;
    uint8_t channelMask;

    chooseKernel(kernelVertical, kernelHorizontal, channelMask);

    cout << "---------" << endl;
    cout << kernelHorizontal << endl;
    cout << "---------" << endl;
    cout << kernelVertical << endl;
    cout << "---------" << endl;
    cout << "Started calcutations..." << endl;


    uint64_t kernelHeight = kernelVertical.getHeight();
    uint64_t kernelWidth = kernelHorizontal.getWidth();

    ImageRowsRingBuffer<Bitmap24Pixel> ringBuffer(kernelHeight, inputWidthPx, paddingBytes);

    if (kernelWidth > inputWidthPx || kernelHeight > inputHeightPx) {
        cerr << "Filter size (" << kernelWidth << " x " << kernelHeight << ") is too big for this image ("
             << inputWidthPx << " x " << inputHeightPx << ")!" << endl;
        return 7;
    }
    outputStream.write((char*)&bitmapOutputInfoHeader, sizeof(BitmapInfoHeaderV3));

    for (int32_t i = 0; i < (kernelHeight + 2) / 2; i++) {
        auto addRow = (char*)ringBuffer.pushNewRowAndGetPtr();
        inputStream.read(addRow, inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes, ios_base::cur);
        ringBuffer.applyHorizontalKernelToLastRow(kernelHorizontal, channelMask);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
            return 8;
        }
        if (i == 0 || (kernelHeight % 2 == 0 && (i == kernelHeight / 2))) {
            continue;
        }
        memcpy((char*)ringBuffer.getRow(kernelHeight - 2 * i - 1), addRow, outputRowBytesCountWithoutPadding);
    }

    Bitmap24Pixel* newRow;
    for (int32_t i = 0; i < inputHeightPx - (kernelHeight + 2) / 2; i++) {
        newRow = ringBuffer.applyVerticalKernel(kernelVertical, channelMask);

        outputStream.write((char*)newRow, outputRowBytesCountWithPadding);
        if (outputStream.fail()) {
            cerr << "Error writing to file!" << endl;
            return 10;
        }

        inputStream.read((char*)ringBuffer.pushNewRowAndGetPtr(), inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes, ios_base::cur);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
            return 11;
        }
        ringBuffer.applyHorizontalKernelToLastRow(kernelHorizontal, channelMask);
    }

    for (int32_t i = 0; i < (kernelHeight + 2) / 2; i++) {
        newRow = ringBuffer.applyVerticalKernel(kernelVertical, channelMask);
        outputStream.write((char*)newRow, outputRowBytesCountWithPadding);
        if (outputStream.fail()) {
            cerr << "Error writing to file!" << endl;
            return 12;
        }
        auto addRow = (char*)ringBuffer.pushNewRowAndGetPtr();
        memcpy(addRow, (char*)ringBuffer.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);
    }


    outputStream.close();
    inputStream.close();
    cout << "Success!" << endl;
    return 0;
}
