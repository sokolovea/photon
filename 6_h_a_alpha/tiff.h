#ifndef TIFF_H
#define TIFF_H
#include <cstdint>
#include <memory>
#include <cmath>
#include <complex>

#define COLORS_NUM UINT16_MAX + 1

struct Tiff16RGBPixel
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

#pragma pack(push, 1)
struct Tiff16ComplexPixel
{
    int16_t real;
    int16_t imag;
};
#pragma pack(pop)

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

#endif // TIFF_H


