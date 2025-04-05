#ifndef WEIGHTSCACHESINGLETON_H
#define WEIGHTSCACHESINGLETON_H

#include <array>
#include <cmath>
#include <cstdint>
#define CACHE_SIZE 600

class WeightsCacheSingleton
{
private:

    WeightsCacheSingleton() {
        double step = 6.0 / CACHE_SIZE;
        for (int i = 0; i < CACHE_SIZE; ++i) {
            double x = -3.0 + i * step;
            lanczosCache[i] = lanczosWeight(x, 3);
        }
    }

    static double sinc(double parX) {
        if (parX == 0) {
            return 1.0;
        }
        return std::sin(M_PI * parX) / (M_PI * parX);
    }

    static double lanczosWeight(double parX, int64_t parA = 3) {
        if (std::abs(parX) >= parA) {
            return 0.0;
        }
        return sinc(parX) * sinc(parX / parA);
    }

public:
    std::array<double, CACHE_SIZE> lanczosCache;

    static WeightsCacheSingleton& getInstance() {
        static WeightsCacheSingleton instance;
        return instance;
    }
};

#endif // WEIGHTSCACHESINGLETON_H
