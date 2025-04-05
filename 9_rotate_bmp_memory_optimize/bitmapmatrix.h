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

struct ChunkInfo {
    double aX;
    double aY;
    double bX;
    double bY;
    double cX;
    double cY;
    double dX;
    double dY;
};

template<typename PixelType = Bitmap24Pixel>
class BitmapOptimizeMatrix {
    std::unique_ptr<Bitmap24Pixel[]> _chunkBitmap;
    int64_t _fullWidth;
    int64_t _fullHeight;

    int64_t _pixelsPerChunkSideOutput;


    int64_t _chunksPerWidthOutput;


    int64_t _chunksPerHeight;


    PixelType _defaultPixelType = PixelTraits<PixelType>::getDefaultPixel();

public:
    int64_t _pixelsPerChunkSideInput;


    int64_t _padding;

    BitmapOptimizeMatrix(int64_t parWidth, int64_t parHeight, int64_t parPixelPerChunkSide = 100) {
        _fullWidth = parWidth;
        _fullHeight = parHeight;

        _pixelsPerChunkSideOutput = parPixelPerChunkSide;
        _pixelsPerChunkSideInput = parPixelPerChunkSide;

        _chunksPerWidthOutput = (parWidth + parPixelPerChunkSide - 1) / parPixelPerChunkSide;
        _chunksPerHeight = (parHeight + parPixelPerChunkSide - 1) / parPixelPerChunkSide;

        // _chunkBitmap = std::make_unique<PixelType[]>(_pixelsPerChunkSideOutput * _pixelsPerChunkSideOutput);
    }

    int64_t getWidth() const {
        return _fullWidth;
    }

    int64_t getHeight() const {
        return _fullHeight;
    }

    PixelType* operator()(uint64_t parRow) {
        return _chunkBitmap.get() + (_padding + _pixelsPerChunkSideInput + _padding) * (parRow + _padding) + _padding;
    }

    PixelType* operator()(int64_t parRow, int64_t parCol) {
        if (parRow >= _pixelsPerChunkSideInput + _padding || parCol >= _pixelsPerChunkSideInput +_padding || parRow < 0 - _padding || parCol < 0 - _padding) {
            return &_defaultPixelType;
        }
        return _chunkBitmap.get() + (_padding + _pixelsPerChunkSideInput + _padding) * (parRow + _padding) + parCol + _padding;
    }

    const ImageNecessaryInfo calculateRotatedImageInfo(double parAlpha, double parZoom, int64_t parPadding) {

        RotateMatrix rotateMatrix = RotateMatrix(parAlpha, 0, 0, parZoom);

        std::pair<double, double> leftDown = rotateMatrix.getNewPixelCoordinates(0, 0);
        std::pair<double, double> leftUpper = rotateMatrix.getNewPixelCoordinates(0, _fullHeight);
        std::pair<double, double> rightDown = rotateMatrix.getNewPixelCoordinates(_fullWidth, 0);
        std::pair<double, double> rightUpper = rotateMatrix.getNewPixelCoordinates(_fullWidth, _fullHeight);

        double minX = std::min(std::min(leftDown.first, leftUpper.first), std::min(rightDown.first, rightUpper.first));
        double minY = std::min(std::min(leftDown.second, leftUpper.second), std::min(rightDown.second, rightUpper.second));
        double maxX = std::max(std::max(leftDown.first, leftUpper.first), std::max(rightDown.first, rightUpper.first));
        double maxY = std::max(std::max(leftDown.second, leftUpper.second), std::max(rightDown.second, rightUpper.second));

        int64_t width = std::round(maxX - minX);
        int64_t height = std::round(maxY - minY);

        double deltaWidth = -minX;
        double deltaHeight = -minY;

        _padding = parPadding;

        _chunksPerWidthOutput = (width + _pixelsPerChunkSideOutput - 1) / _pixelsPerChunkSideOutput;
        _chunksPerHeight = (height + _pixelsPerChunkSideOutput - 1) / _pixelsPerChunkSideOutput;



        _chunkBitmap = std::make_unique<PixelType[]>((_pixelsPerChunkSideInput + 2 * _padding) * (_pixelsPerChunkSideInput + 2 * _padding) * 4 / (parZoom));

        return {
            width,
            height,
            RotateMatrix(parAlpha, deltaWidth, deltaHeight, parZoom)
        };
    }

    const ChunkInfo calculateRowColsInputImage(const ChunkInfo& parChunkInfoOutputImage, const ImageNecessaryInfo& parOutputImageInfo) {


        std::pair<double, double> aPixelCoordinates = parOutputImageInfo.getRotateMatrix().getXYReverseCoordinates(parChunkInfoOutputImage.aX, parChunkInfoOutputImage.aY);
        std::pair<double, double> bPixelCoordinates = parOutputImageInfo.getRotateMatrix().getXYReverseCoordinates(parChunkInfoOutputImage.bX, parChunkInfoOutputImage.bY);
        std::pair<double, double> cPixelCoordinates = parOutputImageInfo.getRotateMatrix().getXYReverseCoordinates(parChunkInfoOutputImage.cX, parChunkInfoOutputImage.cY);
        std::pair<double, double> dPixelCoordinates = parOutputImageInfo.getRotateMatrix().getXYReverseCoordinates(parChunkInfoOutputImage.dX, parChunkInfoOutputImage.dY);
        return {
            aPixelCoordinates.first,
            aPixelCoordinates.second,
            bPixelCoordinates.first,
            bPixelCoordinates.second,
            cPixelCoordinates.first,
            cPixelCoordinates.second,
            dPixelCoordinates.first,
            dPixelCoordinates.second
        };
    }

    void calculateOutputChunk(int64_t parWidth, double parHeight, double parAlpha, double parZoom, const ImageNecessaryInfo& parOutputImageInfo,
                            PixelType* parOutChunkData, InterpolationMode parInterpolationMode = InterpolationMode::NearestNeighbour, int64_t parPadding = 0, double parOffsetX = 0, double parOffsetY = 0) {


        RotateMatrix rotateMatrix = RotateMatrix(parAlpha, 0, 0, parZoom);
        std::pair<double, double> leftDown = rotateMatrix.getNewPixelCoordinates(0, 0);
        std::pair<double, double> leftUpper = rotateMatrix.getNewPixelCoordinates(0, _pixelsPerChunkSideOutput);
        std::pair<double, double> rightDown = rotateMatrix.getNewPixelCoordinates(_pixelsPerChunkSideOutput, 0);
        std::pair<double, double> rightUpper = rotateMatrix.getNewPixelCoordinates(_pixelsPerChunkSideOutput, _pixelsPerChunkSideOutput);

        double minX = std::min(std::min(leftDown.first, leftUpper.first), std::min(rightDown.first, rightUpper.first));
        double minY = std::min(std::min(leftDown.second, leftUpper.second), std::min(rightDown.second, rightUpper.second));


        double deltaWidth = -minX;
        double deltaHeight = -minY;
        _padding = parPadding;

        int currentPixelIndex = -1;
        for (int64_t i = 0; i < parWidth; i++) {
            for (int64_t j = 0; j < parHeight; j++) {
                currentPixelIndex++;
                std::pair<double, double> pixelCoordinates = rotateMatrix.getXYReverseCoordinates(j, i);

                pixelCoordinates.first = pixelCoordinates.first + deltaHeight / parZoom / parZoom + parOffsetX;
                pixelCoordinates.second = pixelCoordinates.second + deltaWidth / parZoom / parZoom + parOffsetY;

                if (parInterpolationMode == InterpolationMode::NearestNeighbour) {
                    parOutChunkData[currentPixelIndex] = *(*this)(static_cast<int64_t>(pixelCoordinates.second),
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

                    parOutChunkData[currentPixelIndex] = PixelTraits<PixelType>::interpolateBilinear(p1, p2, p3, p4, dx, dy);
                } else if (parInterpolationMode == InterpolationMode::Bicubic) {
                    PixelType pixels[4][4];
                    for (int j = -2; j < 2; ++j) {
                        for (int k = -2; k < 2; ++k) {
                            pixels[j + 2][k + 2] = *(*this)(y1 + j, x1 + k);
                        }
                    }
                    parOutChunkData[currentPixelIndex] = PixelTraits<PixelType>::interpolateBicubic(pixels, dx, dy);
                } else if (parInterpolationMode == InterpolationMode::Lanczos3) {
                    PixelType pixels[6][6];
                    for (int j = -2; j <= 3; ++j) {
                        for (int i = -2; i <= 3; ++i) {
                            pixels[j + 2][i + 2] = *(*this)(static_cast<int64_t>(pixelCoordinates.second) + j,
                                                            static_cast<int64_t>(pixelCoordinates.first) + i);
                        }
                    }
                    parOutChunkData[currentPixelIndex] = PixelTraits<PixelType>::interpolateLanczos(pixels, dx, dy);
                }
            }
        }
    }
};

#endif // BITMAPMATRIX_H
