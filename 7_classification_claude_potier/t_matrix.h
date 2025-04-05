#ifndef T_MATRIX_H
#define T_MATRIX_H

#include <eigen3/Eigen/Dense>
#include <complex>

struct TMatrix {
    double E00;
    double E11;
    double E22;
    std::complex<double> E01;
    std::complex<double> E02;
    std::complex<double> E12;

    TMatrix()
        : E00(0), E11(0), E22(0), E01(0, 0), E02(0, 0), E12(0, 0) {}

    TMatrix(double e00, double e11, double e22,
            std::complex<double> e01, std::complex<double> e02, std::complex<double> e12)
        : E00(e00), E11(e11), E22(e22), E01(e01), E02(e02), E12(e12) {}

    Eigen::Matrix3cd getEigenMatrix() {
        Eigen::Matrix3cd T;
        T << E00, E01, E02,
            std::conj(E01), E11, E12,
            std::conj(E02), std::conj(E12), E22;
        return T;
    }

};


#endif // T_MATRIX_H
