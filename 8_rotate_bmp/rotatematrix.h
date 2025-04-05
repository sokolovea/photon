#include <cmath>
#include <cstdint>

#ifndef ROTATEMATRIX_H
#define ROTATEMATRIX_H

class RotateMatrix
{
private:
    double _phi;
    double _deltaWidth;
    double _deltaHeight;
    double _zoom;

public:

    RotateMatrix(): _phi(0), _deltaWidth(0), _deltaHeight(0), _zoom(1) {}

    RotateMatrix(double parPhi, double parDeltaWidth, double parDeltaHeight, double parZoom):
        _phi(parPhi),
        _deltaWidth(parDeltaWidth),
        _deltaHeight(parDeltaHeight),
        _zoom(parZoom) { }

    std::pair<double, double> getNewPixelCoordinates(int64_t parX1, int64_t parY1) const {
        double x2 = _zoom * cos(_phi) * parX1 + _zoom * sin(_phi) * parY1 + _deltaWidth;
        double y2 = -1 * _zoom * sin(_phi) * parX1 + _zoom * cos(_phi) * parY1 + _deltaHeight;
        return {x2, y2};
    }

    // std::pair<double, double> getXYReverseCoordinates(int64_t parX2, int64_t parY2) const {
    //     double y1 = (parY2 - _deltaHeight + (parX2 + _deltaWidth) * tan(_phi)) * cos(_phi) / _zoom;
    //     double x1 = (parX2 - _deltaWidth) / (_zoom * cos(_phi)) - y1 * tan(_phi);
    //     return {x1, y1};
    // }

    std::pair<double, double> getXYReverseCoordinates(int64_t parX2, int64_t parY2) const {
        double x = parX2 - _deltaWidth;
        double y = parY2 - _deltaHeight;
        double x1 = (cos(_phi) * x - sin(_phi) * y) / _zoom;
        double y1 = (sin(_phi) * x + cos(_phi) * y) / _zoom;
        return {x1, y1};
    }
};

#endif // ROTATEMATRIX_H
