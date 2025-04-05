#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#define MAX_KERNEL_SIZE 50

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

    Kernel(): width(0), height(0) {}

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

    double getElem(uint64_t row, uint64_t col) const {
        if (row >= height || col >= width) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[row][col];
    }

    void setElem(uint64_t row, uint64_t col, double value) {
        if (row >= height || col >= width) {
            throw std::out_of_range("Index out of bounds");
        }
        data[row][col] = value;
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

    static Kernel getIdentityKernel(int width, int height) {
        Kernel kernel(width, height);

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                kernel[i][j] = 0;
            }
        }
        kernel[height / 2][width / 2] = 1;

        return kernel;
    }

    static Kernel getAverageKernel(uint64_t width, uint64_t height) {
        Kernel kernel(width, height);
        double value = 1.0 / (width * height);
        for (uint64_t i = 0; i < height; i++) {
            for (uint64_t j = 0; j < width; j++) {
                kernel[i][j] = value;
            }
        }
        return kernel;
    }

    static Kernel getGaussianKernel(uint64_t width, uint64_t height, double stdev) {
        Kernel kernel(width, height);
        double sum = 0;

        for (int64_t i = 0; i < height; ++i) {
            for (int64_t j = 0; j < width; ++j) {
                int64_t x = j - width / 2;
                int64_t y = i - height / 2;
                kernel[i][j] = exp(-(x * x + y * y) / (2 * stdev * stdev)) / (2 * M_PI * stdev * stdev);
                sum += kernel[i][j];
            }
        }

        for (int64_t i = 0; i < height; ++i) {
            for (int64_t j = 0; j < width; ++j) {
                kernel[i][j] /= sum;
            }
        }

        return kernel;
    }

    static Kernel getSharpenKernel(uint64_t width, uint64_t height) {
        Kernel kernel(width, height);
        int centerX = width / 2;
        int centerY = height / 2;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (i == centerY && j == centerX) {
                    kernel[i][j] = width * height;
                } else {
                    kernel[i][j] = -1;
                }
            }
        }

        return kernel;
    }

    static Kernel getSobelVerticalKernel() {
        Kernel kernel(3, 3);
        kernel[0][0] = -1;
        kernel[0][1] = -2;
        kernel[0][2] = -1;
        kernel[1][0] = 0;
        kernel[1][1] = 0;
        kernel[1][2] = 0;
        kernel[2][0] = 1;
        kernel[2][1] = 2;
        kernel[2][2] = 1;
        return kernel;
    }

    static Kernel getSobelHorizontalKernel() {
        Kernel kernel(3, 3);
        kernel[0][0] = -1;
        kernel[0][1] = 0;
        kernel[0][2] = 1;
        kernel[1][0] = -2;
        kernel[1][1] = 0;
        kernel[1][2] = 2;
        kernel[2][0] = -1;
        kernel[2][1] = 0;
        kernel[2][2] = 1;
        return kernel;
    }

};
