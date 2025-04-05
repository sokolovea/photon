#include "bitmap.h"
#include "bitmapmatrix.h"

#include <cstring>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <memory>
#include <vector>

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

    int64_t inputRowBytesCountWithoutPadding = inputWidthPx * 3;

    int64_t inputRowBytesCountWithPadding = getRowSizeWithPadding(inputRowBytesCountWithoutPadding);

    int64_t inputPaddingBytes = inputRowBytesCountWithPadding - inputRowBytesCountWithoutPadding;


    int64_t pixelsPerChunkSideOutput = 100;

    int64_t deltaPadding = 5;


    BitmapOptimizeMatrix inputBitmapMatrix(inputWidthPx, inputHeightPx, pixelsPerChunkSideOutput);

    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);

    ImageNecessaryInfo outputImageInfo = inputBitmapMatrix.calculateRotatedImageInfo(degrees * M_PI / 180, zoom, deltaPadding);

    int64_t outputWidthPx = outputImageInfo.getWidth();
    int64_t outputHeightPx = outputImageInfo.getHeight();

    bitmapOutputInfoHeader.biWidth = outputWidthPx;
    bitmapOutputInfoHeader.biHeight = outputHeightPx;

    int64_t outputRowBytesCountWithoutPadding = outputWidthPx * 3;
    int64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCountWithoutPadding);

    bitmapOutputFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + outputRowBytesCountWithPadding * outputHeightPx;
    bitmapOutputFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);
    outputStream.write((char*)&bitmapOutputFileHeader, sizeof(BitmapFileHeader));
    outputStream.write((char*)&bitmapOutputInfoHeader, sizeof(BitmapInfoHeaderV3));
    std::unique_ptr<Bitmap24Pixel[]> outputChunk = std::make_unique<Bitmap24Pixel[]>(pixelsPerChunkSideOutput * pixelsPerChunkSideOutput);


    auto chunksPerWidthOutput = (outputWidthPx + pixelsPerChunkSideOutput - 1) / pixelsPerChunkSideOutput;
    auto chunksPerHeight = (outputHeightPx + pixelsPerChunkSideOutput - 1) / pixelsPerChunkSideOutput;
    int64_t chunksCount = chunksPerWidthOutput * chunksPerHeight;





    ChunkInfo chunkInfoOutput;

    size_t bufferSize = outputRowBytesCountWithPadding;
    vector<char>buffer(bufferSize, 0);

    for (size_t i = 0; i < outputHeightPx; ++i) {
        if (outputStream.fail()) {
            cerr << "Can't init output file!" << endl;
            return 6;
        }
        outputStream.write(buffer.data(), bufferSize);
    }

    for (int64_t i = 0; i < chunksCount ; i++) {

        chunkInfoOutput.aX = (i % chunksPerWidthOutput) * pixelsPerChunkSideOutput;
        chunkInfoOutput.bX = (i % chunksPerWidthOutput) * pixelsPerChunkSideOutput + pixelsPerChunkSideOutput;
        chunkInfoOutput.cX = (i % chunksPerWidthOutput) * pixelsPerChunkSideOutput;
        chunkInfoOutput.dX = (i % chunksPerWidthOutput) * pixelsPerChunkSideOutput + pixelsPerChunkSideOutput;


        chunkInfoOutput.aY = (i / chunksPerWidthOutput) * pixelsPerChunkSideOutput;
        chunkInfoOutput.bY = (i / chunksPerWidthOutput) * pixelsPerChunkSideOutput;
        chunkInfoOutput.cY = (i / chunksPerWidthOutput) * pixelsPerChunkSideOutput + pixelsPerChunkSideOutput;
        chunkInfoOutput.dY = (i / chunksPerWidthOutput) * pixelsPerChunkSideOutput + pixelsPerChunkSideOutput;

        ChunkInfo inputChunkInfo = inputBitmapMatrix.calculateRowColsInputImage(chunkInfoOutput, outputImageInfo);

        double minXDouble = std::min(std::min(inputChunkInfo.aX, inputChunkInfo.bX), std::min(inputChunkInfo.cX, inputChunkInfo.dX));
        double minYDouble = std::min(std::min(inputChunkInfo.aY, inputChunkInfo.bY), std::min(inputChunkInfo.cY, inputChunkInfo.dY));
        double maxXDouble = std::max(std::max(inputChunkInfo.aX, inputChunkInfo.bX), std::max(inputChunkInfo.cX, inputChunkInfo.dX));
        double maxYDouble = std::max(std::max(inputChunkInfo.aY, inputChunkInfo.bY), std::max(inputChunkInfo.cY, inputChunkInfo.dY));


        int64_t minX = floor(minXDouble) - deltaPadding;
        int64_t minY = floor(minYDouble) - deltaPadding;
        int64_t maxY = ceil(maxYDouble) + deltaPadding;
        int64_t maxX = ceil(maxXDouble) + deltaPadding;

        int64_t width = maxX - minX;
        int64_t height = maxY - minY;

        int rowCounter = -1;

        inputBitmapMatrix._pixelsPerChunkSideInput = width;
        inputBitmapMatrix._padding = 0;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                auto pixel = inputBitmapMatrix(x, y);
                pixel->red = 255;
                pixel->green = 255;
                pixel->blue = 255;
            }
        }



        for (int64_t j = minY; j < maxY; j++) {
            rowCounter++;
            int64_t row = inputHeightPx - j - 1;

            if (row < 0 || row >= inputHeightPx) {
                continue;
            }

            int64_t col = minX >= 0 ? minX : 0;
            int64_t xPadding = minX >= 0 ? 0 : -minX;

            int64_t bytesToRead = (width - xPadding) * 3;
            int64_t bytesToRowEnd = std::max(inputRowBytesCountWithoutPadding - minX * 3, 0L);

            inputStream.seekg(bitmapInputFileHeader.bfOffBits + row * inputRowBytesCountWithPadding + (col) * 3, std::ios::beg);

            if (bytesToRead > 0) {
                inputStream.read(reinterpret_cast<char*>(inputBitmapMatrix(rowCounter) + xPadding), std::min(bytesToRead, bytesToRowEnd));
            }
        }

        if (inputStream.fail()) {
            std::cerr << "Error reading source file!" << std::endl;
            return 8;
        }


        width -= 2 * deltaPadding;
        height -= 2 * deltaPadding;
        inputBitmapMatrix._pixelsPerChunkSideInput = width;

        inputBitmapMatrix.calculateOutputChunk(pixelsPerChunkSideOutput, pixelsPerChunkSideOutput, degrees * M_PI / 180, zoom, outputImageInfo, outputChunk.get(), interpolationMode, deltaPadding,
                                                 (-minX + minXDouble - deltaPadding),  (-minY + minYDouble - deltaPadding));


        for (int64_t y = 0; y < pixelsPerChunkSideOutput; y++) {
            int64_t outputRow = outputImageInfo.getHeight() - ((chunkInfoOutput.aY) + y) - 1; // Перевод в BMP-координаты

            if (outputRow < 0 || outputRow >= outputImageInfo.getHeight()) {
                continue;
            }

            int64_t fileOffset = bitmapOutputFileHeader.bfOffBits + outputRow * outputRowBytesCountWithPadding + chunkInfoOutput.aX * 3;
            outputStream.seekp(fileOffset, std::ios::beg);

            int64_t currentWidth = std::min(std::abs(outputWidthPx - (int64_t)chunkInfoOutput.aX), pixelsPerChunkSideOutput);
            outputStream.write(reinterpret_cast<char*>(outputChunk.get() + y * pixelsPerChunkSideOutput), currentWidth * 3);

        }
    }

    return 0;

}
