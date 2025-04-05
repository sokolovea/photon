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

struct BitmapInfoHeader
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint8_t payload[24];
};

#pragma pack(push, 1)
struct Bitmap24Pixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
#pragma pack(pop)

// #pragma pack(push, 1)
// struct Bitmap8Pixel
// {
//     uint8_t data;
// };
// #pragma pack(pop)