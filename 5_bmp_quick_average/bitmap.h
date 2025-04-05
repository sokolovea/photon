#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct BitmapFileHeader
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
#pragma pack(pop)

struct BitmapInfoHeaderV3
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

#pragma pack(push, 1)
struct Bitmap24Pixel
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;

    static uint64_t getMaxChannelValue() {
        return 255;
    }
};
#pragma pack(pop)

struct Bitmap32Pixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};


struct Bitmap24Image
{
    BitmapFileHeader bitmapFileHeader;
    BitmapInfoHeaderV3 bitmapInfoHeaderV3;
    // std::unique_ptr<uint8_t[]> pixels;
};

inline uint64_t getRowSizeWithPadding(uint64_t rowSizeWithoutPadding) {
    return ((rowSizeWithoutPadding + 3) &~ 3);
}

template<typename T>
inline T divideWithCeil(T a, T b) {
    return (a + b - 1) / b;
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
