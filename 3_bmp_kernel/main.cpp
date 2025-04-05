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

void chooseKernel(Kernel& kernel, uint8_t& channelMask) {
    kernel = Kernel::getIdentityKernel(3, 3);

    cout << "Choose kernel:" << endl;
    cout << "1. Manual" << endl;
    cout << "2. Identity" << endl;
    cout << "3. Average" << endl;
    cout << "4. Gaussian" << endl;
    cout << "5. Sharpen" << endl;
    cout << "6. SobelVertical" << endl;
    cout << "7. SobelHorizontal" << endl;

    int64_t chosenNumber;
    uint64_t rows;
    uint64_t cols;
    double stdev;
    cin >> chosenNumber;

    if (chosenNumber <= 0 || chosenNumber > 7) {
        throw std::invalid_argument("Invalid number selection!");
    }

    channelMask = ImageChannel::Red | ImageChannel::Green | ImageChannel::Blue;

    if (chosenNumber != 6 && chosenNumber != 7) {
        cout << "Input rows and cols count: " << endl;
        cin >> rows;
        cin >> cols;
    } else {
        rows = 3;
        cols = 3;
    }

    kernel.setHeight(rows);
    kernel.setWidth(rows);

    if (chosenNumber == 1) { // Manual
        cout << "Input matrix: " << endl;
        for (int64_t i = 0; i < rows; i++) {
            for (int64_t j = 0; j < cols; j++) {
                do {
                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                    cout << "Kernel[" << i + 1 << "][" << j + 1 << "] = ";
                    cin >> kernel[i][j];
                } while(cin.fail());
            }
        }
    }
    else if (chosenNumber == 2) { // Identity
        kernel = Kernel::getIdentityKernel(cols, rows);
    }
    else if (chosenNumber == 3) { // Average
        kernel = Kernel::getAverageKernel(cols, rows);
    }
    else if (chosenNumber == 4) { // Gaussian
        cout << "Input stdev: " << endl;
        cin >> stdev;
        kernel = Kernel::getGaussianKernel(cols, rows, stdev);
    }
    else if (chosenNumber == 5) { // Sharpen
        kernel = Kernel::getSharpenKernel(cols, rows);
    }
    else if (chosenNumber == 6) { // Sobel vertical
        kernel = Kernel::getSobelVerticalKernel();
    }
    else if (chosenNumber == 7) { // Sobel horizontal
        kernel = Kernel::getSobelHorizontalKernel();
    } else {
        throw std::invalid_argument("Invalid kernel selection!");
    }
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

    Kernel kernel;
    uint8_t channelMask;
    try {
        chooseKernel(kernel, channelMask);
    } catch (const std::exception& exception) {
        cout << "Fatal error!" << endl << exception.what() << endl;
        return 6;
    }

    cout << "---------" << endl;
    cout << kernel << endl;
    cout << "---------" << endl;
    cout << "Started calcutations..." << endl;


    uint64_t kernelHeight = kernel.getHeight();
    uint64_t kernelWidth = kernel.getWidth();

    ImageRowsRingBuffer<Bitmap24Pixel> ringBuffer(kernelHeight, inputWidthPx, paddingBytes);

    if (kernelWidth > inputWidthPx || kernelHeight > inputHeightPx) {
        cerr << "Filter size (" << kernelWidth << " x " << kernelHeight << ") is too big for this image ("
             << inputWidthPx << " x " << inputHeightPx << ")!" << endl;
        return 7;
    }
    outputStream.write((char*)&bitmapOutputInfoHeader, sizeof(BitmapInfoHeaderV3));

    inputStream.seekg((kernelHeight + 1) / 2 * inputRowBytesCountWithPadding, ios_base::cur);
    for (int32_t i = 0; i < (kernelHeight + 1) / 2; i++) {
        inputStream.read((char*)ringBuffer.pushNewRowAndGetPtr(), inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes - 2 * inputRowBytesCountWithPadding, ios_base::cur);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
            return 8;
        }
    }

    inputStream.seekg(bitmapOutputFileHeader.bfOffBits, ios_base::beg);

    for (int32_t i = 0; i < (kernelHeight) / 2; i++) {
        inputStream.read((char*)ringBuffer.pushNewRowAndGetPtr(), inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes, ios_base::cur);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
            return 9;
        }
    }

    Bitmap24Pixel* newRow;
    for (int32_t i = 0; i < inputHeightPx - (kernelHeight) / 2; i++) {
        inputStream.read((char*)ringBuffer.pushNewRowAndGetPtr(), inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes, ios_base::cur);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
            return 10;
        }

        newRow = ringBuffer.applyKernel(kernel, channelMask);

        outputStream.write((char*)newRow, outputRowBytesCountWithPadding);
        if (outputStream.fail()) {
            cerr << "Error writing to file!" << endl;
            return 11;
        }
    }

    for (int32_t i = 0; i < (kernelHeight) / 2; i++) {
        inputStream.seekg(paddingBytes - 2 * inputRowBytesCountWithPadding, ios_base::cur);
        inputStream.read((char*)ringBuffer.pushNewRowAndGetPtr(), inputRowBytesCountWithoutPadding);
        newRow = ringBuffer.applyKernel(kernel, channelMask);
        outputStream.write((char*)newRow, outputRowBytesCountWithPadding);
        if (outputStream.fail()) {
            cerr << "Error writing to file!" << endl;
            return 12;
        }
    }


    outputStream.close();
    inputStream.close();
    cout << "Success!" << endl;
    return 0;
}
