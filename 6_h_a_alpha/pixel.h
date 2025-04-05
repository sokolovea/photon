#pragma once
#include <cstdint>
#include <cmath>
#include <complex>
#include "imageutils.h"

#pragma pack(push, 1)
struct RGB8Pixel
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct OneChannelAbsComplexPixel
{
    double channel;

    OneChannelAbsComplexPixel() {}

    OneChannelAbsComplexPixel(double value) {
        channel = value;
    }

};
#pragma pack(pop)


#pragma pack(push, 1)
struct Complex16Pixel
{
    double real;
    double imag;

    Complex16Pixel() {}

    Complex16Pixel(std::complex<double> value) {
        real = value.real();
        imag = value.imag();
    }

    Complex16Pixel(double real, double imag) {
        this->real = real;
        this->imag = imag;
    }

    void setValue(std::complex<double> value) {
        real = value.real();
        imag = value.imag();
    }

};
#pragma pack(pop)


class RGB8Statistics {
public:
    double red;
    double green;
    double blue;
};

class OneChannelComplex16Statistics {
public:
    double real;
    double imag;
    OneChannelComplex16Statistics() {
        real = 0;
        imag = 0;
    }

    OneChannelComplex16Statistics(double real, double imag) {
        this->real = real;
        this->imag = imag;
    }
};

class OneChannel16Statistics {
public:
    double channel;
    OneChannel16Statistics() {
        channel = 0;
    }
    OneChannel16Statistics(double channel) {
        this->channel = channel;
    }
};


template <typename PixelStructure>
class PixelStatisticsTraits {

};

template <>
class PixelStatisticsTraits<RGB8Statistics> {
public:
    static void reset(RGB8Statistics& pixel) {
        pixel.red = 0;
        pixel.green = 0;
        pixel.blue = 0;
    }

    static void add(RGB8Statistics& target, const RGB8Pixel& source) {
        target.red += source.red;
        target.green += source.green;
        target.blue += source.blue;
    }

    static void subtract(RGB8Statistics& target, const RGB8Pixel& source) {
        target.red -= source.red;
        target.green -= source.green;
        target.blue -= source.blue;
    }

    static RGB8Pixel normalize(const RGB8Statistics& RGB8Statistics, int divisor) {
        return {
            .blue = static_cast<uint8_t>(RGB8Statistics.blue / divisor),
            .green = static_cast<uint8_t>(RGB8Statistics.green / divisor),
            .red = static_cast<uint8_t>(RGB8Statistics.red / divisor)
        };
    }
};


template <>
class PixelStatisticsTraits<OneChannel16Statistics> {
public:
    static void reset(OneChannel16Statistics& pixel) {
        pixel.channel = 0;
    }

    static void add(OneChannel16Statistics& target, const OneChannelAbsComplexPixel& source) {
        target.channel += source.channel;
    }

    static void subtract(OneChannel16Statistics& target, const OneChannelAbsComplexPixel& source) {
        target.channel -= source.channel;
    }

    static OneChannelAbsComplexPixel normalize(const OneChannel16Statistics& pixel, int divisor) {
        return OneChannelAbsComplexPixel(pixel.channel / divisor);
    }
};

template <>
class PixelStatisticsTraits<OneChannelComplex16Statistics> {
public:
    static void reset(OneChannelComplex16Statistics& pixel) {
        pixel.real = 0;
        pixel.imag = 0;
    }

    static void add(OneChannelComplex16Statistics& target, const Complex16Pixel& source) {
        target.real += source.real;
        target.imag += source.imag;
    }

    static void subtract(OneChannelComplex16Statistics& target, const Complex16Pixel& source) {
        target.real -= source.real;
        target.imag -= source.imag;
    }

    static Complex16Pixel normalize(const OneChannelComplex16Statistics& pixel, int divisor) {
        return Complex16Pixel(
            pixel.real / divisor,
            pixel.imag / divisor
        );
    }
};
