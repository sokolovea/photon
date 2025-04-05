#ifndef IMAGENECESSARYINFO_H
#define IMAGENECESSARYINFO_H

#include "rotatematrix.h"
#include <cstdint>
class ImageNecessaryInfo
{
    int64_t _width;
    int64_t _height;
    RotateMatrix _rotateMatrix;

public:
    ImageNecessaryInfo(int64_t parWidth, int64_t parHeight, RotateMatrix parRotateMatrix) {
        _width = parWidth;
        _height = parHeight;
        _rotateMatrix = parRotateMatrix;
    }

    const int64_t getWidth() const {
        return _width;
    }

    const int64_t getHeight() const {
        return _height;
    }

    const RotateMatrix& getRotateMatrix() const {
        return _rotateMatrix;
    }
};

#endif // IMAGENECESSARYINFO_H
