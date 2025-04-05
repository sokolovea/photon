#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <memory>
#include "bitmap.h"
#include "imageutils.h"
#include "tiff.h"

using namespace std;

enum class WorkMode {
    INCREASE,
    DECREASE
};

Bitmap24Image getBitmap24ImageWithFilledHeaders(int32_t width, int32_t height);

int main(int argc, char** argv) {
    if (argc != 7) {
        cerr << "Wrong parameters count!" << endl;
        return 1;
    }

    WorkMode workMode;
    if (!strcmp("-dec", argv[1])) {
        workMode = WorkMode::DECREASE;
    } else {
        cerr << "Unknown work mode \"" << argv[1] << "\"!" << endl;
        return 2;
    }


    int32_t scaleCoeff = std::atoi(argv[2]);
    if (scaleCoeff <= 0) {
        cerr << "Coefficient is not correct!" << endl;
        return 3;
    }

    double discardedPixelFractionCoeffMin;
    double discardedPixelFractionCoeffMax;
    try {
        discardedPixelFractionCoeffMin = std::stod(argv[5]);
        discardedPixelFractionCoeffMax = std::stod(argv[6]);
        if (discardedPixelFractionCoeffMin < 0 || discardedPixelFractionCoeffMin > 1 ||
            discardedPixelFractionCoeffMax < 0 || discardedPixelFractionCoeffMax > 1 ) {
            std::cerr << "Discarded pixel fraction coefficient is not correct!" << std::endl;
            return 3;
        }
    } catch (...) {
        std::cerr << "Discarded pixel fraction coefficient is not correct!" << std::endl;
        return 3;
    }

    Tiff16RGBImage tiffImage;
    tiffImage.inputStream.open(argv[3], std::ios_base::binary);

    ofstream outputStream;
    outputStream.open(argv[4], std::ios_base::binary);

    if (!tiffImage.inputStream.is_open() || !outputStream.is_open()) {
        cerr << "Can't open files!" << endl;
        return 4;
    }

    TiffFileHeader tiffInputFileHeader;
    tiffImage.inputStream.read((char*)&tiffInputFileHeader, sizeof(TiffFileHeader));
    if (tiffInputFileHeader.byteOrder != 0x4949) {
        cerr << "Not little-endian TIFF!" << endl;
        return 5;
    }

    if (tiffInputFileHeader.versionNumber != 42) {
        cerr << "Unknown TIFF version!" << endl;
        return 6;
    }

    tiffImage.inputStream.seekg(tiffInputFileHeader.offsetIFD, std::ios::beg);


    uint16_t numberEntries;
    tiffImage.inputStream.read((char*)&numberEntries, sizeof(uint16_t));
    if (tiffImage.inputStream.fail()) {
        cerr << "Error reading source image!" << endl;
        return 7;
    };
    TiffIFD tiffIDF(numberEntries);
    tiffImage.inputStream.read((char*)tiffIDF.entries.get(), numberEntries * sizeof(TiffIFDEntry));
    if (tiffImage.inputStream.fail()) {
        cerr << "Error reading source image!" << endl;
        return 8;
    };

    for (int i = 0; i < numberEntries; i++) {
        auto entry = tiffIDF.entries.get()[i];
        auto tag = tiffIDF.entries.get()[i].tag;
        tiffImage.setValueByKey(tag, tiffIDF.entries.get()[i].valueOffset, tiffIDF.entries.get()[i].numberValues);
        cout << "Tag = " << tag << "; numberValues = " << entry.numberValues;
        if (entry.numberValues == 1) {
            cout << "; value = " << entry.valueOffset << endl;
        } else {
            cout << "; values = ";
            try {
                auto data = tiffImage.getValuesVector(entry.tag, entry.valueOffset, entry.numberValues,  static_cast<FieldType>(entry.fieldType));
                for (int j = 0; j < entry.numberValues; j++) {
                    cout << data[j] << " ";
                    if (j > 3) {
                        cout << "...";
                        break;
                    }
                }
                std::cout << std::endl;
            } catch (const std::runtime_error& e) {
                cerr << "Error reading values by key!" << endl;
                tiffImage.inputStream.clear();
                continue;
            }
        }
    }
    std::cout << "------------" << std::endl;

    try {
        tiffImage.checkAndPrintNecessaryInformation();    
    } catch (const std::runtime_error& e) {
        cerr << "Tiff image is not valid: " << e.what() << endl;
        return 9;
    }

    int32_t inputWidthPx = tiffImage.width;
    int32_t inputHeightPx = tiffImage.height;

    int32_t outputWidthPx = workMode == WorkMode::INCREASE? inputWidthPx * scaleCoeff : divideWithCeil(inputWidthPx, scaleCoeff);
    int32_t outputHeight = workMode == WorkMode::INCREASE? inputHeightPx * scaleCoeff : divideWithCeil(inputHeightPx, scaleCoeff);

    Bitmap24Image bitmap24Image = getBitmap24ImageWithFilledHeaders(outputWidthPx, outputHeight);

    uint64_t outputRowBytesCount = outputWidthPx * sizeof(Bitmap24Pixel);
    uint64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCount);

    uint64_t inputRowBytesCount = inputWidthPx * sizeof(Tiff16RGBPixel);

    bitmap24Image.bitmapFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + outputRowBytesCount * outputHeight;
    bitmap24Image.bitmapFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);
    outputStream.write((char*)&bitmap24Image.bitmapFileHeader, sizeof(BitmapFileHeader));

    bitmap24Image.bitmapInfoHeaderV3.biWidth = outputWidthPx;
    bitmap24Image.bitmapInfoHeaderV3.biHeight = outputHeight;
    outputStream.write((char*)&bitmap24Image.bitmapInfoHeaderV3, sizeof(BitmapInfoHeaderV3));

    std::cout << "------------" << std::endl;
    std::cout << "Started statistics evaluating..." << std::endl << std::endl;
    Tiff16RGBImageStatistics tiffStatistics = tiffImage.getStatistics(discardedPixelFractionCoeffMin, discardedPixelFractionCoeffMax);
    std::cout << "Brief statistics:" << std::endl;
    std::cout << tiffStatistics << std::endl;

    ColorsConstraits colorsConstraits;

    colorsConstraits.red.first = tiffStatistics.red.leftBorder;
    colorsConstraits.red.second = tiffStatistics.red.rightBorder;

    colorsConstraits.green.first = tiffStatistics.green.leftBorder;
    colorsConstraits.green.second = tiffStatistics.green.rightBorder;

    colorsConstraits.blue.first = tiffStatistics.blue.leftBorder;
    colorsConstraits.blue.second = tiffStatistics.blue.rightBorder;

    std::cout << "------------" << std::endl;
    std::cout << "Recalcutated borders:" << std::endl;
    std::cout << colorsConstraits << std::endl;


    auto pixelConvertCallback = [&colorsConstraits] (Tiff16RGBPixel& inputPixel, Bitmap24Pixel& outputPixel)
    {
        convertPixelFormat(inputPixel, outputPixel, UINT8_MAX, colorsConstraits);
    };

    std::vector<char> zeroBuffer(outputRowBytesCountWithPadding, 0);
    for (uint64_t i = 0; i < outputHeight; i++) {
        outputStream.write(zeroBuffer.data(), outputRowBytesCountWithPadding);
        if (tiffImage.inputStream.fail()) {
            cerr << "Error creating target image file!" << endl;
            return 10;
        }
    }

    std::unique_ptr<uint8_t[]> inputRow = std::make_unique<uint8_t[]>(inputRowBytesCount);
    std::unique_ptr<uint8_t[]> outputRow = std::make_unique<uint8_t[]>(outputRowBytesCountWithPadding);
    auto stripOffsets = tiffImage.stripOffsets;
    tiffImage.inputStream.seekg(stripOffsets[0], ios_base::beg);
    if (tiffImage.inputStream.fail()) {
        cerr << "Error reading source image!" << endl;
        return 11;
    };
    uint32_t rowsPerStrip = tiffImage.rowsPerStrip;
    for (uint64_t i = 0; i < outputHeight; i++) {
        uint32_t currentStrip = i * scaleCoeff / rowsPerStrip;
        uint32_t currentRowInStrip = i * scaleCoeff % rowsPerStrip;
        tiffImage.inputStream.read((char*)inputRow.get(), inputRowBytesCount);
        if (tiffImage.inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
            return 6;
        }
        tiffImage.inputStream.seekg(stripOffsets[currentStrip] + currentRowInStrip * inputRowBytesCount, ios_base::beg);
        decreaseResolution((Tiff16RGBPixel*)inputRow.get(), (Bitmap24Pixel*)outputRow.get(), inputWidthPx,
                           outputWidthPx, scaleCoeff, pixelConvertCallback);
        outputStream.seekp(-1 * outputRowBytesCountWithPadding, std::ios::cur);
        outputStream.write((char*)outputRow.get(), outputRowBytesCountWithPadding);
        if (outputStream.fail()) {
            cerr << "Error writing to file!" << endl;
            return 13;
        }
        outputStream.seekp(-1 * outputRowBytesCountWithPadding, std::ios::cur);
    }

    outputStream.close();
    tiffImage.inputStream.close();
    cout << "Successfully!" << endl;
    return 0;
}

Bitmap24Image getBitmap24ImageWithFilledHeaders(int32_t width, int32_t height) {
    Bitmap24Image bitmap24Image;
    int rowSizeWithoutPadding = width * sizeof(Bitmap24Pixel);
    int rowSizeWithPadding = getRowSizeWithPadding(rowSizeWithoutPadding);
    int imageSize = rowSizeWithPadding * height;

    bitmap24Image.bitmapFileHeader.bfType = 0x4d42;
    bitmap24Image.bitmapFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + imageSize;
    bitmap24Image.bitmapFileHeader.bfReserved1 = 0;
    bitmap24Image.bitmapFileHeader.bfReserved2 = 0;
    bitmap24Image.bitmapFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);

    bitmap24Image.bitmapInfoHeaderV3.biWidth = width;
    bitmap24Image.bitmapInfoHeaderV3.biHeight = height;
    bitmap24Image.bitmapInfoHeaderV3.biSize = sizeof(BitmapInfoHeaderV3);
    bitmap24Image.bitmapInfoHeaderV3.biPlanes = 1;
    bitmap24Image.bitmapInfoHeaderV3.biBitCount = 24;
    bitmap24Image.bitmapInfoHeaderV3.biCompression = 0;
    bitmap24Image.bitmapInfoHeaderV3.biSizeImage = imageSize;
    bitmap24Image.bitmapInfoHeaderV3.biXPelsPerMeter = 2835;
    bitmap24Image.bitmapInfoHeaderV3.biYPelsPerMeter = 2835;
    bitmap24Image.bitmapInfoHeaderV3.biClrUsed = 0;
    bitmap24Image.bitmapInfoHeaderV3.biClrImportant = 0;

    return bitmap24Image;
}
