#ifndef PIXELTRAITS_H
#define PIXELTRAITS_H

#include "weightscachesingleton.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

template <typename PixelType>
struct PixelTraits {
    using ChannelType = decltype(PixelType{}.red);

    const static WeightsCacheSingleton weights;

    static PixelType getDefaultPixel() {
        return PixelType(
            PixelType::getMaxChannelValue(),
            PixelType::getMaxChannelValue(),
            PixelType::getMaxChannelValue()
        );
    }

    static PixelType interpolateBilinear(const PixelType& parPixel1, const PixelType& parPixel2,
                                         const PixelType& parPixel3, const PixelType& parPixel4,
                                         double parDx, double parDy) {
        auto interpolate = [](ChannelType c1, ChannelType c2, ChannelType c3,
                              ChannelType c4, double dx, double dy) {
            return static_cast<ChannelType>(
                (1 - dx) * (1 - dy) * c1 +
                dx * (1 - dy) * c2 +
                (1 - dx) * dy * c3 +
                dx * dy * c4
            );
        };

        return {
            interpolate(parPixel1.red, parPixel2.red, parPixel3.red, parPixel4.red, parDx, parDy),
            interpolate(parPixel1.green, parPixel2.green, parPixel3.green, parPixel4.green, parDx, parDy),
            interpolate(parPixel1.blue, parPixel2.blue, parPixel3.blue, parPixel4.blue, parDx, parDy)
        };
    }

    static double cubicWeight(double parX, double parA = -0.5) {
        parX = std::abs(parX);
        if (parX < 1) {
            return (parA + 2) * parX * parX * parX - (parA + 3) * parX * parX + 1;
        } else if (parX < 2) {
            return parA * parX * parX * parX - 5 * parA * parX * parX + 8 * parA * parX - 4 * parA;
        }
        return 0.0;
    }

    static PixelType interpolateBicubic(const PixelType parPixels[4][4], double parDx, double parDy) {
        auto interpolateChannel = [&](auto channelGetter) {
            double result = 0;
            for (int64_t j = 0; j < 4; ++j) {
                double weightY = cubicWeight(parDy - (j - 1));
                double rowResult = 0;
                for (int64_t i = 0; i < 4; ++i) {
                    double weightX = cubicWeight(parDx - (i - 1));
                    rowResult += weightX * channelGetter(parPixels[j][i]);
                }
                result += weightY * rowResult;
            }
            return static_cast<typename PixelType::ChannelType>(std::clamp(result, 0.0, static_cast<double>(PixelType::getMaxChannelValue())));
        };

        return {
            interpolateChannel([](const PixelType& p) { return p.red; }),
            interpolateChannel([](const PixelType& p) { return p.green; }),
            interpolateChannel([](const PixelType& p) { return p.blue; })
        };
    }

    // static double sinc(double parX) {
    //     if (parX == 0) {
    //         return 1.0;
    //     }
    //     return std::sin(M_PI * parX) / (M_PI * parX);
    // }

    // static double lanczosWeight(double parX, int64_t parA = 3) {
    //     if (std::abs(parX) >= parA) {
    //         return 0.0;
    //     }
    //     return sinc(parX) * sinc(parX / parA);
    // }

    static double getLanczosWeight(double x) {
        if (x <= -3.0 || x >= 3.0) return 0.0;
        constexpr double step = 6.0 / CACHE_SIZE;
        int index = static_cast<int>((x + 3.0) / step);
        return weights.lanczosCache[index];
    }

    static PixelType interpolateLanczos(const PixelType parPixels[6][6],
                                        double parDx, double parDy, int64_t parA = 3) {
        auto interpolateChannel = [&](auto channelGetter) {
            double result = 0.0;
            double sumWeight = 0.0;
            for (int64_t j = -parA + 1; j <= parA; j++) {
                for (int64_t i = -parA + 1; i <= parA; i++) {
                    double weight = getLanczosWeight(parDx - i) * getLanczosWeight(parDy - j); //lanczosWeight(parDy - j, parA);
                    result += weight * channelGetter(parPixels[j + parA - 1][i + parA - 1]);
                    sumWeight += weight;

                    // std::cout << parDx - i << std::endl;
                }
            }
            return static_cast<typename PixelType::ChannelType>(
                std::clamp(result / sumWeight, 0.0, static_cast<double>(PixelType::getMaxChannelValue()))
            );
        };

        return {
            interpolateChannel([](const PixelType& p) { return p.red; }),
            interpolateChannel([](const PixelType& p) { return p.green; }),
            interpolateChannel([](const PixelType& p) { return p.blue; })
        };
    }
};

template <typename PixelType>
const WeightsCacheSingleton PixelTraits<PixelType>::weights = WeightsCacheSingleton::getInstance();

#endif // PIXELTRAITS_H
