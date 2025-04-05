#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>

#define MAX_KERNEL_SIZE 100

class Kernel
{
private:
    double data[MAX_KERNEL_SIZE][MAX_KERNEL_SIZE];
    int64_t width;
    int64_t height;
    uint64_t getValidatedSize(uint64_t size) {
        if (size > MAX_KERNEL_SIZE) {
            return MAX_KERNEL_SIZE;
        }
        if (size == 0) {
            return 1;
        }
        return size;
    }
public:

    Kernel(): width(1), height(1) {}

    Kernel(uint64_t width, uint64_t height) {
        this->width = getValidatedSize(width);
        this->height = getValidatedSize(height);
    }

    void setWidth(uint64_t width) {
        this->width = getValidatedSize(width);
    }

    void setHeight(uint64_t height) {
        this->height = getValidatedSize(height);
    }

    double* operator[](uint64_t row) {
        return data[row];
    }

    const double* operator[](uint64_t row) const {
        return data[row];
    }

    int64_t getWidth() const {
        return width;
    }

    int64_t getHeight() const {
        return height;
    }

    friend std::ostream& operator<<(std::ostream& os, const Kernel& kernel) {
        os << "Kernel (" << kernel.height << "x" << kernel.width << "):" << std::endl;
        for (int64_t i = 0; i < kernel.height; i++) {
            for (int64_t j = 0; j < kernel.width; j++) {
                os << std::setw(8) << std::setprecision(5) << kernel.data[i][j] << " ";
            }
            os << std::endl;
        }
        return os;
    }

    void setToGaussianKernel(double stdev) {
        double sum = 0;
        for (int64_t i = 0; i < height; ++i) {
            for (int64_t j = 0; j < width; ++j) {
                int64_t x = j - width / 2;
                int64_t y = i - height / 2;
                data[i][j] = exp(-(x * x + y * y) / (2 * stdev * stdev)) / (2 * M_PI * stdev * stdev);
                sum += data[i][j];
            }
        }

        for (int64_t i = 0; i < height; ++i) {
            for (int64_t j = 0; j < width; ++j) {
                data[i][j] /= sum;
            }
        }
    }


};
