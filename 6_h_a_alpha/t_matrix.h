#ifndef T_MATRIX_H
#define T_MATRIX_H

#include <complex>

struct TMatrix {
    double E00;
    double E11;
    double E22;
    std::complex<double> E01;
    std::complex<double> E02;
    std::complex<double> E12;
};

#endif // T_MATRIX_H
