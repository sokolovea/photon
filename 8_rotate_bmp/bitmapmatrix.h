#ifndef BITMAPMATRIX_H
#define BITMAPMATRIX_H

#include <cmath>
#include <memory>
#include "pixeltraits.h"
#include "bitmap.h"
#include "imagenecessaryinfo.h"
#include "rotatematrix.h"

enum class InterpolationMode
{
    NearestNeighbour,
    Bilinear,
    Bicubic,
    Lanczos3
};

template<typename PixelType = Bitmap24Pixel>
class BitmapMatrix {
    std::unique_ptr<Bitmap24Pixel[]> _bitmap;
    int64_t _width;
    int64_t _height;
    PixelType _defaultPixelType = PixelTraits<PixelType>::getDefaultPixel();

public:
    BitmapMatrix(int64_t parWidth, int64_t parHeight) {
        _width = parWidth;
        _height = parHeight;
        _bitmap = std::make_unique<PixelType[]>(parWidth * parHeight);
    }

    uint32_t getWidth() const {
        return _width;
    }

    uint32_t getHeight() const {
        return _height;
    }

    PixelType* operator()(uint64_t parRow) {
        return _bitmap.get() + _width * parRow;
    }

    PixelType* operator()(int64_t parRow, int64_t parCol) {
        parRow = mirrorCoordinate(parRow, _height);
        parCol = mirrorCoordinate(parCol, _width);
        if (parRow >= getHeight() || parCol >= getWidth() || parRow < 0 || parCol < 0) {
            return &_defaultPixelType;
        }
        return _bitmap.get() + _width * parRow + parCol;
    }

    int64_t mirrorCoordinate(int64_t parIndex, int64_t parMaxIndex) {
        if (parIndex < 0) {
            return -parIndex;
        }
        while (parIndex > 2 * parMaxIndex) {
            parIndex -= parIndex;
        }
        if (parIndex >= parMaxIndex) {
            return 2 * parMaxIndex - parIndex - 2;
        }
        return parIndex;
    }

    const ImageNecessaryInfo getRotatedImageInfo(double parAlpha, double parZoom) const {

        RotateMatrix rotateMatrix = RotateMatrix(parAlpha, 0, 0, parZoom);

        std::pair<double, double> leftDown = rotateMatrix.getNewPixelCoordinates(0, 0);
        std::pair<double, double> leftUpper = rotateMatrix.getNewPixelCoordinates(0, _height);
        std::pair<double, double> rightDown = rotateMatrix.getNewPixelCoordinates(_width, 0);
        std::pair<double, double> rightUpper = rotateMatrix.getNewPixelCoordinates(_width, _height);

        double minX = std::min(std::min(leftDown.first, leftUpper.first), std::min(rightDown.first, rightUpper.first));
        double minY = std::min(std::min(leftDown.second, leftUpper.second), std::min(rightDown.second, rightUpper.second));
        double maxX = std::max(std::max(leftDown.first, leftUpper.first), std::max(rightDown.first, rightUpper.first));
        double maxY = std::max(std::max(leftDown.second, leftUpper.second), std::max(rightDown.second, rightUpper.second));

        int64_t width = std::ceil(maxX - minX);
        int64_t height = std::ceil(maxY - minY);

        double deltaWidth = -minX;
        double deltaHeight = -minY;

        return {
            width,
            height,
            RotateMatrix(parAlpha, deltaWidth, deltaHeight, parZoom)
        };
    }

    void calculateOutputRow(int64_t parRow, const ImageNecessaryInfo& parOutputImageInfo,
                            PixelType* parOutPixelRow, InterpolationMode parInterpolationMode = InterpolationMode::Bicubic) {
        for (int64_t i = 0; i < parOutputImageInfo.getWidth(); i++) {
            std::pair<double, double> pixelCoordinates = parOutputImageInfo.getRotateMatrix().getXYReverseCoordinates(i, parRow);
            double x = pixelCoordinates.first;
            double y = pixelCoordinates.second;

            if ((int)y < 0 || (int)x < 0 || (int)y >= getHeight() || (int)x >= getWidth()) {
                parOutPixelRow[i] = _defaultPixelType;
                continue;
            }

            if (parInterpolationMode == InterpolationMode::NearestNeighbour) {
                parOutPixelRow[i] = *(*this)(static_cast<int64_t>(pixelCoordinates.second),
                                             static_cast<int64_t>(pixelCoordinates.first));
                continue;
            }
            int x1 = floor(pixelCoordinates.first);
            int y1 = floor(pixelCoordinates.second);
            int x2 = ceil(pixelCoordinates.first);
            int y2 = ceil(pixelCoordinates.second);
            double dx = pixelCoordinates.first - x1;
            double dy = pixelCoordinates.second - y1;

            if (parInterpolationMode == InterpolationMode::Bilinear) {

                PixelType p1 = *(*this)(y1, x1);
                PixelType p2 = *(*this)(y1, x2);
                PixelType p3 = *(*this)(y2, x1);
                PixelType p4 = *(*this)(y2, x2);


                parOutPixelRow[i] = PixelTraits<PixelType>::interpolateBilinear(p1, p2, p3, p4, dx, dy);
            } else if (parInterpolationMode == InterpolationMode::Bicubic) {
                PixelType pixels[4][4];
                for (int j = -2; j < 2; ++j) {
                    for (int k = -2; k < 2; ++k) {
                        pixels[j + 2][k + 2] = *(*this)(y1 + j, x1 + k);
                    }
                }
                parOutPixelRow[i] = PixelTraits<PixelType>::interpolateBicubic(pixels, dx, dy);
            } else if (parInterpolationMode == InterpolationMode::Lanczos3) {
                PixelType pixels[6][6];
                for (int j = -2; j <= 3; ++j) {
                    for (int i = -2; i <= 3; ++i) {
                        pixels[j + 2][i + 2] = *(*this)(static_cast<int64_t>(pixelCoordinates.second) + j,
                                                        static_cast<int64_t>(pixelCoordinates.first) + i);
                    }
                }
                parOutPixelRow[i] = PixelTraits<PixelType>::interpolateLanczos(pixels, dx, dy);
            }

        }
    }
};

#endif // BITMAPMATRIX_H
