#ifndef TIFFIMAGEREADER_H
#define TIFFIMAGEREADER_H

#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <vector>
#include "tiff.h"
#include <stdexcept>
#include <iostream>

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

template <typename T>
class TiffImageReader {
public:
    TiffImageReader(const std::string& filename) : filename(filename), currentRow(0) {}

    ~TiffImageReader() {
        if (inputStream.is_open()) {
            inputStream.close();
        }
    }

    bool open() {
        inputStream.open(filename, std::ios_base::binary);
        if (!inputStream.is_open()) {
            std::cerr << "Can't open file!" << std::endl;
            return false;
        }
        loadTiffHeader();
        loadIFD();
        this->rowBytesCount = this->width * sizeof(T);
        return true;
    }

    bool readNextRow(T* rowBuffer) {
        if (currentRow >= height) {
            return false;
        }

        uint32_t currentStrip = currentRow / rowsPerStrip;
        uint32_t currentRowInStrip = currentRow % rowsPerStrip;
        inputStream.seekg(stripOffsets[currentStrip] + currentRowInStrip * rowBytesCount, std::ios_base::beg);
        inputStream.read(reinterpret_cast<char*>(rowBuffer), rowBytesCount);

        if (inputStream.fail()) {
            std::cerr << "Error reading source image!" << std::endl;
            return false;
        }

        currentRow++;
        return true;
    }

    bool isOpen() const {
        return inputStream.is_open();
    }

    int32_t getWidth() const {
        return width;
    }

    int32_t getHeight() const {
        return height;
    }

    uint32_t getCurrentRow() const {
        return currentRow;
    }

private:
    std::string filename;
    std::ifstream inputStream;
    int32_t width = 0;
    int32_t height = 0;
    int64_t rowBytesCount = 0;
    int32_t rowsPerStrip = 0;
    std::vector<uint64_t> stripOffsets;
    int32_t currentRow = 0;

    void loadTiffHeader() {
        TiffFileHeader tiffFileHeader;
        inputStream.read(reinterpret_cast<char*>(&tiffFileHeader), sizeof(TiffFileHeader));
        if (tiffFileHeader.byteOrder != 0x4949) {
            std::cerr << "Not little-endian TIFF!" << std::endl;
            throw std::runtime_error("Invalid byte order");
        }
        if (tiffFileHeader.versionNumber != 42) {
            std::cerr << "Unknown TIFF version!" << std::endl;
            throw std::runtime_error("Unknown TIFF version");
        }
        inputStream.seekg(tiffFileHeader.offsetIFD, std::ios::beg);
    }

    void loadIFD() {
        uint16_t numberEntries;
        inputStream.read(reinterpret_cast<char*>(&numberEntries), sizeof(uint16_t));
        TiffIFD tiffIFD(numberEntries);
        inputStream.read(reinterpret_cast<char*>(tiffIFD.entries.get()), numberEntries * sizeof(TiffIFDEntry));

        for (int i = 0; i < numberEntries; i++) {
            auto& entry = tiffIFD.entries.get()[i];
            if (entry.tag == TiffTagEnum::ImageWidth) {
                width = entry.valueOffset;
            } else if (entry.tag == TiffTagEnum::ImageLength) {
                height = entry.valueOffset;
            } else if (entry.tag == TiffTagEnum::RowsPerStrip) {
                rowsPerStrip = entry.valueOffset;
            } else if (entry.tag == TiffTagEnum::StripOffsets) {
                stripOffsets = getValuesVector(entry.tag, entry.valueOffset, entry.numberValues, FieldType::Long);
            }
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
};

#endif // TIFFIMAGEREADER_H
