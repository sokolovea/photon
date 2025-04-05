#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <eigen3/Eigen/Dense>
#include "bitmap.h"
#include "imageutils.h"
#include "tiff.h"
#include "tiffimagereader.h"
#include "imagerowsringbuffer.h"
#include "pixel.h"
#include "t_matrix.h"

using namespace std;


Bitmap24Image getBitmap24ImageWithFilledHeaders(int32_t width, int32_t height);

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Wrong params count!" << endl;
        return 1;
    }
    ifstream inputConfig(argv[1]);
    if (!inputConfig.is_open()) {
        cerr << "Can not open config file!" << endl;
        return 2;
    }
    auto imagesFileNames = std::make_unique<string[]>(4);
    for (int64_t i = 0; i < 4; i++) {
        if (!getline(inputConfig, imagesFileNames[i]) || imagesFileNames[i].empty()) {
            cerr << "Error reading file name from config file!" << endl;
            return 3;
        }
    }
    TiffImageReader<Tiff16ComplexPixel> tiffReaderHH(imagesFileNames[0]);
    TiffImageReader<Tiff16ComplexPixel> tiffReaderHV(imagesFileNames[1]);
    TiffImageReader<Tiff16ComplexPixel> tiffReaderVH(imagesFileNames[2]);
    TiffImageReader<Tiff16ComplexPixel> tiffReaderVV(imagesFileNames[3]);
    if (!tiffReaderHH.open() || !tiffReaderHV.open() || !tiffReaderVH.open() || !tiffReaderVV.open()) {
        cerr << "Can't open files!" << endl;
        return 4;
    }

    if (tiffReaderHH.getWidth() != tiffReaderHV.getWidth() ||
        tiffReaderHH.getWidth() != tiffReaderVH.getWidth() ||
        tiffReaderHH.getWidth() != tiffReaderVV.getWidth() ||
        tiffReaderHH.getHeight() != tiffReaderHV.getHeight() ||
        tiffReaderHH.getHeight() != tiffReaderVH.getHeight() ||
        tiffReaderHH.getHeight() != tiffReaderVV.getHeight()) {
        cerr << "The sizes of the input images are not equivalent!" << endl;
        return 5;
    }

    int32_t outputWidthPx =  tiffReaderHH.getWidth();
    int32_t outputHeightPx = tiffReaderHH.getHeight();

    cout << "Image size: " << outputWidthPx << " x " << outputHeightPx << endl;

    auto rowBufferHH = std::make_unique<Tiff16ComplexPixel[]>(outputWidthPx);
    auto rowBufferHV = std::make_unique<Tiff16ComplexPixel[]>(outputWidthPx);
    auto rowBufferVH = std::make_unique<Tiff16ComplexPixel[]>(outputWidthPx);
    auto rowBufferVV = std::make_unique<Tiff16ComplexPixel[]>(outputWidthPx);

    Bitmap24Image bitmap24Image = getBitmap24ImageWithFilledHeaders(outputWidthPx, outputHeightPx);

    int64_t outputRowBytesCountWithoutPadding = outputWidthPx * sizeof(Bitmap24Pixel);
    int64_t outputCustomTMatrixRowBytesCount = outputWidthPx * sizeof(TMatrix);
    int64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCountWithoutPadding);
    int64_t padding = outputRowBytesCountWithPadding - outputRowBytesCountWithoutPadding;

    ofstream outputStream;
    outputStream.open(argv[2], std::ios_base::binary);
    if (!outputStream.is_open()) {
        cerr << "Can not open output file!" << endl;
        return 6;
    }

    ofstream outputStreamRawT;
    outputStreamRawT.open(string(argv[2]).append(".raw"), std::ios_base::binary);
    if (!outputStreamRawT.is_open()) {
        cerr << "Can not open output file with raw T data!" << endl;
        return 6;
    }

    bitmap24Image.bitmapFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + outputRowBytesCountWithoutPadding * outputHeightPx;
    bitmap24Image.bitmapFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);
    outputStream.write((char*)&bitmap24Image.bitmapFileHeader, sizeof(BitmapFileHeader));

    bitmap24Image.bitmapInfoHeaderV3.biWidth = outputWidthPx;
    bitmap24Image.bitmapInfoHeaderV3.biHeight = outputHeightPx;
    outputStream.write((char*)&bitmap24Image.bitmapInfoHeaderV3, sizeof(BitmapInfoHeaderV3));

    auto rowBufferOutputBmp = std::make_unique<Bitmap24Pixel[]>(outputWidthPx);

    double sqrt2 = std::sqrt(2.0);

    std::vector<uint8_t> zeroBuffer(outputRowBytesCountWithPadding);
    for (int64_t i = 0; i < tiffReaderHH.getHeight(); i++) {
        outputStream.write((char*)zeroBuffer.data(), outputRowBytesCountWithPadding);
    }

    outputStream.seekp(-outputRowBytesCountWithPadding, ios_base::cur);

    Eigen::Matrix3cd T;
    Eigen::ArrayXd alphaCoeff(3);

    int64_t kernelHeight = 7;
    int64_t kernelWidth = 7;

    auto rowCustomTMatrix = std::make_unique<TMatrix[]>(outputWidthPx);

    ImageRowsRingBuffer<OneChannelAbsComplexPixel, OneChannel16Statistics> ringBufferAlphaSquare(kernelHeight, outputWidthPx, 0);
    ImageRowsRingBuffer<Complex16Pixel, OneChannelComplex16Statistics> ringBufferAlphaBetaConj(kernelHeight, outputWidthPx, 0);
    ImageRowsRingBuffer<Complex16Pixel, OneChannelComplex16Statistics> ringBufferAlphaGammaConj(kernelHeight, outputWidthPx, 0);
    ImageRowsRingBuffer<OneChannelAbsComplexPixel, OneChannel16Statistics> ringBufferBetaSquare(kernelHeight, outputWidthPx, 0);
    ImageRowsRingBuffer<Complex16Pixel, OneChannelComplex16Statistics> ringBufferBetaGammaConj(kernelHeight, outputWidthPx, 0);
    ImageRowsRingBuffer<OneChannelAbsComplexPixel, OneChannel16Statistics> ringBufferGammaSquare(kernelHeight, outputWidthPx, 0);



    TiffImageReader<Tiff16RGBPixel> standartTiff = TiffImageReader<Tiff16RGBPixel>("standart.tiff");
    if (!standartTiff.open()) {
        cout << "Failed to read standart file!" << endl;
        return 6;
    }

    std::unique_ptr<Tiff16RGBPixel[]> standartRow = std::make_unique<Tiff16RGBPixel[]>(outputWidthPx);

    for (int32_t i = 0; i < (kernelHeight + 2) / 2; i++) {
        bool isSuccess = true;
        isSuccess &= tiffReaderHH.readNextRow(rowBufferHH.get());
        isSuccess &= tiffReaderHV.readNextRow(rowBufferHV.get());
        isSuccess &= tiffReaderVH.readNextRow(rowBufferVH.get());
        isSuccess &= tiffReaderVV.readNextRow(rowBufferVV.get());
        if (!isSuccess) {
            cerr << "Failed to read rows from input files!" << endl;
            return 7;
        }
        cout << "Reading row " << tiffReaderHH.getCurrentRow() << endl;

        auto alphaSquareRingBufferRow = ringBufferAlphaSquare.pushNewRowAndGetPtr();
        auto alphaBetaConjRingBufferRow = ringBufferAlphaBetaConj.pushNewRowAndGetPtr();
        auto alphaGammaConjRingBufferRow = ringBufferAlphaGammaConj.pushNewRowAndGetPtr();
        auto betaSquareRingBufferRow = ringBufferBetaSquare.pushNewRowAndGetPtr();
        auto betaGammaConjRingBufferRow = ringBufferBetaGammaConj.pushNewRowAndGetPtr();
        auto gammaSquareRingBufferRow = ringBufferGammaSquare.pushNewRowAndGetPtr();

        for (int j = 0; j < outputWidthPx; j++) {
            std::complex<double> SHH = std::complex<double>(rowBufferHH[j].real, rowBufferHH[j].imag);
            std::complex<double> SVV = std::complex<double>(rowBufferVV[j].real, rowBufferVV[j].imag);
            std::complex<double> SHV = std::complex<double>(rowBufferHV[j].real, rowBufferHV[j].imag);
            std::complex<double> SVH = std::complex<double>(rowBufferVH[j].real, rowBufferVH[j].imag);

            auto S_alpha = divideComplexByReal(SHH + SVV, sqrt2);
            auto S_beta = divideComplexByReal(SHH - SVV,  sqrt2);
            auto S_gamma = divideComplexByReal(SHV + SVH, sqrt2);

            OneChannelAbsComplexPixel E00 = complexSquare(S_alpha);
            OneChannelAbsComplexPixel E11 = complexSquare(S_beta);
            OneChannelAbsComplexPixel E22 = complexSquare(S_gamma);
            std::complex<double> E01 = S_alpha * std::conj(S_beta);
            std::complex<double> E02 = S_alpha * std::conj(S_gamma);
            std::complex<double> E12 = S_beta * std::conj(S_gamma);

            alphaSquareRingBufferRow[j] = E00;
            alphaBetaConjRingBufferRow[j].setValue(E01);
            alphaGammaConjRingBufferRow[j].setValue(E02);
            betaSquareRingBufferRow[j] = E11;
            betaGammaConjRingBufferRow[j].setValue(E12);
            gammaSquareRingBufferRow[j] = E22;

        }

        ringBufferAlphaSquare.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferAlphaBetaConj.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferAlphaGammaConj.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferBetaSquare.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferBetaGammaConj.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferGammaSquare.applyHorizontalKernelToLastRow(kernelWidth);

        if (i == 0 || (kernelHeight % 2 == 0 && (i == kernelHeight / 2))) {
            continue;
        }
        memcpy((char*)ringBufferAlphaSquare.getRow(kernelHeight - 2 * i - 1), alphaSquareRingBufferRow, outputRowBytesCountWithoutPadding);
        memcpy((char*)ringBufferAlphaBetaConj.getRow(kernelHeight - 2 * i - 1), alphaBetaConjRingBufferRow, outputRowBytesCountWithoutPadding);
        memcpy((char*)ringBufferAlphaGammaConj.getRow(kernelHeight - 2 * i - 1), alphaGammaConjRingBufferRow, outputRowBytesCountWithoutPadding);
        memcpy((char*)ringBufferBetaSquare.getRow(kernelHeight - 2 * i - 1), betaSquareRingBufferRow, outputRowBytesCountWithoutPadding);
        memcpy((char*)ringBufferBetaGammaConj.getRow(kernelHeight - 2 * i - 1), betaGammaConjRingBufferRow, outputRowBytesCountWithoutPadding);
        memcpy((char*)ringBufferGammaSquare.getRow(kernelHeight - 2 * i - 1), gammaSquareRingBufferRow, outputRowBytesCountWithoutPadding);
    }

    ringBufferAlphaSquare.updateFullColsBuffer();
    ringBufferAlphaBetaConj.updateFullColsBuffer();
    ringBufferAlphaGammaConj.updateFullColsBuffer();
    ringBufferBetaSquare.updateFullColsBuffer();
    ringBufferBetaGammaConj.updateFullColsBuffer();
    ringBufferGammaSquare.updateFullColsBuffer();

    for (int32_t i = 0; i < outputHeightPx - (kernelHeight + 2) / 2; i++) {
        auto alphaSquareRingBufferRow = ringBufferAlphaSquare.applyVerticalKernel(kernelHeight);
        auto alphaBetaConjRingBufferRow = ringBufferAlphaBetaConj.applyVerticalKernel(kernelHeight);
        auto alphaGammaConjRingBufferRow = ringBufferAlphaGammaConj.applyVerticalKernel(kernelHeight);
        auto betaSquareRingBufferRow = ringBufferBetaSquare.applyVerticalKernel(kernelHeight);
        auto betaGammaConjRingBufferRow = ringBufferBetaGammaConj.applyVerticalKernel(kernelHeight);
        auto gammaSquareRingBufferRow = ringBufferGammaSquare.applyVerticalKernel(kernelHeight);

        bool standartReadResult = standartTiff.readNextRow(standartRow.get());
        if (!standartReadResult) {
            cout << "Can't read new row from standart.tiff!" << endl;
        }

        for (int j = 0; j < outputWidthPx; j++) {

            double E00 = alphaSquareRingBufferRow[j].channel;
            double E11 = betaSquareRingBufferRow[j].channel;
            double E22 = gammaSquareRingBufferRow[j].channel;
            std::complex<double> E01(alphaBetaConjRingBufferRow[j].real, alphaBetaConjRingBufferRow[j].imag);
            std::complex<double> E02(alphaGammaConjRingBufferRow[j].real, alphaGammaConjRingBufferRow[j].imag);
            std::complex<double> E12(betaGammaConjRingBufferRow[j].real, betaGammaConjRingBufferRow[j].imag);

            T(0, 0) = E00;
            T(1, 1) = E11;
            T(2, 2) = E22;
            T(0, 1) = E01;
            T(1, 0) = std::conj(E01);
            T(0, 2) = E02;
            T(2, 0) = std::conj(E02);
            T(1, 2) = E12;
            T(2, 1) = std::conj(E12);

            rowCustomTMatrix[j].E00 = E00;
            rowCustomTMatrix[j].E11 = E11;
            rowCustomTMatrix[j].E22 = E22;
            rowCustomTMatrix[j].E01 = E01;
            rowCustomTMatrix[j].E02 = E02;
            rowCustomTMatrix[j].E12 = E12;




            Eigen::SelfAdjointEigenSolver<Eigen::Matrix3cd> solver(T);
            Eigen::Matrix3cd eigenVectors = solver.eigenvectors();
            Eigen::Vector3d eigenValues = solver.eigenvalues();

            std::array<int, 3> indices = {0, 1, 2};
            std::sort(indices.begin(), indices.end(), [&](int a, int b) {
                return std::abs(eigenValues(a)) > std::abs(eigenValues(b));
            });

            Eigen::Vector3d sortedEigenValues;
            Eigen::Matrix3cd sortedEigenVectors;
            for (int i = 0; i < 3; ++i) {
                sortedEigenValues(i) = eigenValues(indices[i]);
                sortedEigenVectors.col(i) = eigenVectors.col(indices[i]);
            }


            double totalSum = sortedEigenValues.cwiseAbs().sum();
            Eigen::Vector3d probabilities = sortedEigenValues.cwiseAbs() / totalSum;

            double H = -1.0 * (probabilities.array() * (probabilities.array().log() / std::log(3.0))).sum();

            double A = (probabilities(1) - probabilities(2)) / (probabilities(1) + probabilities(2));

            // std::cout << "Normalized probabilities: " << probabilities.cwiseAbs().sum() << std::endl;

            alphaCoeff[0] = std::acos(std::abs(sortedEigenVectors(0, 0)));
            alphaCoeff[1] = std::acos(std::abs(sortedEigenVectors(0, 1)));
            alphaCoeff[2] = std::acos(std::abs(sortedEigenVectors(0, 2)));
            double alpha = (alphaCoeff * probabilities.array()).sum();


            Bitmap24Pixel& pixel = rowBufferOutputBmp[j];

            pixel.green = getValueInInterval(H, 0.0, 1.0, 255);
            // cout << A << endl;
            pixel.blue = getValueInInterval(A,  0.0, 1.0, 255);
            pixel.red = getValueInInterval(alpha, 0.0, M_PI_2, 255);

            // if (j >= 1100 && i > 100 && i < 6000) {

            //     if (i == 5000) {
            //         cout << "---" << endl;
            //         cout << "pixel.red = " << (int)pixel.red << "; " << standartRow[j].red * 65535 / 9000.0 / 255 << endl;
            //         cout << "pixel.green = " << (int)pixel.green << "; " << standartRow[j].green * 65535 / 10000.0 / 255 << endl;
            //         cout << "pixel.blue = " << (int)pixel.blue << "; " << standartRow[j].blue * 65535 / 10000.0 / 255 << endl;
            //     }

            //     rowBufferOutputBmp[j].red = standartRow[j].red * 65535 / 9000.0 / 255;
            //     rowBufferOutputBmp[j].green = standartRow[j].green * 65535 / 10000.0 / 255;
            //     rowBufferOutputBmp[j].blue = standartRow[j].blue * 65535 / 10000.0 / 255;
            // }

        }
        if (i % 100 == 0) {
            cout << "Reading row " << tiffReaderHH.getCurrentRow() << endl;
        }
        outputStream.write((char*)rowBufferOutputBmp.get(), outputRowBytesCountWithoutPadding);
        outputStream.seekp(-2 * outputRowBytesCountWithPadding + padding, ios_base::cur);

        outputStreamRawT.write((char*)rowCustomTMatrix.get(), outputCustomTMatrixRowBytesCount);

        ringBufferAlphaSquare.updateSumColsBufferByRow(0, -1);
        ringBufferAlphaBetaConj.updateSumColsBufferByRow(0, -1);
        ringBufferAlphaGammaConj.updateSumColsBufferByRow(0, -1);
        ringBufferBetaSquare.updateSumColsBufferByRow(0, -1);
        ringBufferBetaGammaConj.updateSumColsBufferByRow(0, -1);
        ringBufferGammaSquare.updateSumColsBufferByRow(0, -1);

        bool isSuccess = true;
        isSuccess &= tiffReaderHH.readNextRow(rowBufferHH.get());
        isSuccess &= tiffReaderHV.readNextRow(rowBufferHV.get());
        isSuccess &= tiffReaderVH.readNextRow(rowBufferVH.get());
        isSuccess &= tiffReaderVV.readNextRow(rowBufferVV.get());
        if (!isSuccess) {
            cerr << "Failed to read row!" << endl;
            return 8;
        }

        alphaSquareRingBufferRow = ringBufferAlphaSquare.pushNewRowAndGetPtr();
        alphaBetaConjRingBufferRow = ringBufferAlphaBetaConj.pushNewRowAndGetPtr();
        alphaGammaConjRingBufferRow = ringBufferAlphaGammaConj.pushNewRowAndGetPtr();
        betaSquareRingBufferRow = ringBufferBetaSquare.pushNewRowAndGetPtr();
        betaGammaConjRingBufferRow = ringBufferBetaGammaConj.pushNewRowAndGetPtr();
        gammaSquareRingBufferRow = ringBufferGammaSquare.pushNewRowAndGetPtr();

        for (int j = 0; j < outputWidthPx; j++) {
            std::complex<double> SHH = std::complex<double>(rowBufferHH[j].real, rowBufferHH[j].imag);
            std::complex<double> SVV = std::complex<double>(rowBufferVV[j].real, rowBufferVV[j].imag);
            std::complex<double> SHV = std::complex<double>(rowBufferHV[j].real, rowBufferHV[j].imag);
            std::complex<double> SVH = std::complex<double>(rowBufferVH[j].real, rowBufferVH[j].imag);

            auto S_alpha = divideComplexByReal(SHH + SVV, sqrt2);
            auto S_beta = divideComplexByReal(SHH - SVV,  sqrt2);
            auto S_gamma = divideComplexByReal(SHV + SVH, sqrt2);

            OneChannelAbsComplexPixel E00 = complexSquare(S_alpha);
            OneChannelAbsComplexPixel E11 = complexSquare(S_beta);
            OneChannelAbsComplexPixel E22 = complexSquare(S_gamma);
            std::complex<double> E01 = S_alpha * std::conj(S_beta);
            std::complex<double> E02 = S_alpha * std::conj(S_gamma);
            std::complex<double> E12 = S_beta * std::conj(S_gamma);

            alphaSquareRingBufferRow[j] = E00;
            alphaBetaConjRingBufferRow[j].setValue(E01);
            alphaGammaConjRingBufferRow[j].setValue(E02);
            betaSquareRingBufferRow[j] = E11;
            betaGammaConjRingBufferRow[j].setValue(E12);
            gammaSquareRingBufferRow[j] = E22;

        }

        ringBufferAlphaSquare.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferAlphaBetaConj.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferAlphaGammaConj.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferBetaSquare.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferBetaGammaConj.applyHorizontalKernelToLastRow(kernelWidth);
        ringBufferGammaSquare.applyHorizontalKernelToLastRow(kernelWidth);

        ringBufferAlphaSquare.updateSumColsBufferByRow(kernelWidth - 1, 1);
        ringBufferAlphaBetaConj.updateSumColsBufferByRow(kernelWidth - 1, 1);
        ringBufferAlphaGammaConj.updateSumColsBufferByRow(kernelWidth - 1, 1);
        ringBufferBetaSquare.updateSumColsBufferByRow(kernelWidth - 1, 1);
        ringBufferBetaGammaConj.updateSumColsBufferByRow(kernelWidth - 1, 1);
        ringBufferGammaSquare.updateSumColsBufferByRow(kernelWidth - 1, 1);
    }



    for (int32_t i = 0; i < (kernelHeight + 2) / 2; i++) {
        auto alphaSquareRingBufferRow = ringBufferAlphaSquare.applyVerticalKernel(kernelHeight);
        auto alphaBetaConjRingBufferRow = ringBufferAlphaBetaConj.applyVerticalKernel(kernelHeight);
        auto alphaGammaConjRingBufferRow = ringBufferAlphaGammaConj.applyVerticalKernel(kernelHeight);
        auto betaSquareRingBufferRow = ringBufferBetaSquare.applyVerticalKernel(kernelHeight);
        auto betaGammaConjRingBufferRow = ringBufferBetaGammaConj.applyVerticalKernel(kernelHeight);
        auto gammaGammaSquareRingBufferRow = ringBufferGammaSquare.applyVerticalKernel(kernelHeight);

        for (int j = 0; j < outputWidthPx; j++) {

            double E00 = alphaSquareRingBufferRow[j].channel;
            double E11 = betaSquareRingBufferRow[j].channel;
            double E22 = gammaGammaSquareRingBufferRow[j].channel;
            std::complex<double> E01(alphaBetaConjRingBufferRow[j].real, alphaBetaConjRingBufferRow[j].imag);
            std::complex<double> E02(alphaGammaConjRingBufferRow[j].real, alphaGammaConjRingBufferRow[j].imag);
            std::complex<double> E12(betaGammaConjRingBufferRow[j].real, betaGammaConjRingBufferRow[j].imag);

            T(0, 0) = E00;
            T(1, 1) = E11;
            T(2, 2) = E22;
            T(0, 1) = E01;
            T(1, 0) = std::conj(E01);
            T(0, 2) = E02;
            T(2, 0) = std::conj(E02);
            T(1, 2) = E12;
            T(2, 1) = std::conj(E12);

            rowCustomTMatrix[j].E00 = E00;
            rowCustomTMatrix[j].E11 = E11;
            rowCustomTMatrix[j].E22 = E22;
            rowCustomTMatrix[j].E01 = E01;
            rowCustomTMatrix[j].E02 = E02;
            rowCustomTMatrix[j].E12 = E12;


            Eigen::SelfAdjointEigenSolver<Eigen::Matrix3cd> solver(T);
            Eigen::Matrix3cd eigenVectors = solver.eigenvectors();
            Eigen::Vector3d eigenValues = solver.eigenvalues();

            std::array<int, 3> indices = {0, 1, 2};
            std::sort(indices.begin(), indices.end(), [&](int a, int b) {
                return eigenValues(a) > eigenValues(b);
            });

            Eigen::Vector3d sortedEigenValues;
            Eigen::Matrix3cd sortedEigenVectors;
            for (int i = 0; i < 3; ++i) {
                sortedEigenValues(i) = eigenValues(indices[i]);
                sortedEigenVectors.col(i) = eigenVectors.col(indices[i]);
            }


            double totalSum = sortedEigenValues.cwiseAbs().sum();
            Eigen::Vector3d probabilities = sortedEigenValues.cwiseAbs() / totalSum;

            double H = -1.0 * (probabilities.array() * (probabilities.array().log() / std::log(3.0))).sum();

            double A = (probabilities(1) - probabilities(2)) / (probabilities(1) + probabilities(2));


            alphaCoeff[0] = std::acos(std::abs(sortedEigenVectors(0, 0)));
            alphaCoeff[1] = std::acos(std::abs(sortedEigenVectors(0, 1)));
            alphaCoeff[2] = std::acos(std::abs(sortedEigenVectors(0, 2)));
            double alpha = (alphaCoeff * probabilities.array()).sum();

            Bitmap24Pixel& pixel = rowBufferOutputBmp[j];

            pixel.green = getValueInInterval(H, 0.0, 1.0, 255);
            pixel.blue = getValueInInterval(A, 0.0, 1.0, 255);
            pixel.red = getValueInInterval(alpha, 0.0, M_PI_2, 255);
        }

        cout << "Processing buffered row" << endl;
        outputStream.write((char*)rowBufferOutputBmp.get(), outputRowBytesCountWithoutPadding);
        if (outputStream.fail()) {
            cerr << "Error writing to file!" << endl;
            return 9;
        }
        if (i != (kernelHeight) / 2) {
            outputStream.seekp(-2 * outputRowBytesCountWithPadding + padding, ios_base::cur);
        }

        outputStreamRawT.write((char*)rowCustomTMatrix.get(), outputCustomTMatrixRowBytesCount);

        alphaSquareRingBufferRow = ringBufferAlphaSquare.pushNewRowAndGetPtr();
        alphaBetaConjRingBufferRow = ringBufferAlphaBetaConj.pushNewRowAndGetPtr();
        alphaGammaConjRingBufferRow = ringBufferAlphaGammaConj.pushNewRowAndGetPtr();
        betaSquareRingBufferRow = ringBufferBetaSquare.pushNewRowAndGetPtr();
        betaGammaConjRingBufferRow = ringBufferBetaGammaConj.pushNewRowAndGetPtr();
        gammaGammaSquareRingBufferRow = ringBufferGammaSquare.pushNewRowAndGetPtr();

        ringBufferAlphaSquare.updateSumColsBufferByRow(kernelHeight - 1, -1);
        ringBufferAlphaBetaConj.updateSumColsBufferByRow(kernelHeight - 1, -1);
        ringBufferAlphaGammaConj.updateSumColsBufferByRow(kernelHeight - 1, -1);
        ringBufferBetaSquare.updateSumColsBufferByRow(kernelHeight - 1, -1);
        ringBufferBetaGammaConj.updateSumColsBufferByRow(kernelHeight - 1, -1);
        ringBufferGammaSquare.updateSumColsBufferByRow(kernelHeight - 1, -1);

        memcpy(alphaSquareRingBufferRow, (char*)ringBufferAlphaSquare.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);
        memcpy(alphaBetaConjRingBufferRow, (char*)ringBufferAlphaBetaConj.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);
        memcpy(alphaGammaConjRingBufferRow, (char*)ringBufferAlphaGammaConj.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);
        memcpy(betaSquareRingBufferRow, (char*)ringBufferBetaSquare.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);
        memcpy(betaGammaConjRingBufferRow, (char*)ringBufferBetaGammaConj.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);
        memcpy(gammaGammaSquareRingBufferRow, (char*)ringBufferGammaSquare.getRow(kernelHeight - 2 * i), outputRowBytesCountWithoutPadding);

        ringBufferAlphaSquare.updateSumColsBufferByRow(kernelHeight - 2 * i, 1);
        ringBufferAlphaBetaConj.updateSumColsBufferByRow(kernelHeight - 2 * i, 1);
        ringBufferAlphaGammaConj.updateSumColsBufferByRow(kernelHeight - 2 * i, 1);
        ringBufferBetaSquare.updateSumColsBufferByRow(kernelHeight - 2 * i, 1);
        ringBufferBetaGammaConj.updateSumColsBufferByRow(kernelHeight - 2 * i, 1);
        ringBufferGammaSquare.updateSumColsBufferByRow(kernelHeight - 2 * i, 1);

    }
    cout << "Done!" << endl;
    return 0;
}



Bitmap24Image getBitmap24ImageWithFilledHeaders(int32_t width, int32_t height) {
    Bitmap24Image bitmap24Image;
    int rowSizeWithoutPadding = width * sizeof(Bitmap24Pixel);
    int rowSizeWithPadding = getRowSizeWithPadding(rowSizeWithoutPadding);
    int imageSize = rowSizeWithPadding * height;

    bitmap24Image.bitmapFileHeader.bfType = 0x4d42;
    bitmap24Image.bitmapFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + imageSize;
    bitmap24Image.bitmapFileHeader.bfReserved1 = 0;
    bitmap24Image.bitmapFileHeader.bfReserved2 = 0;
    bitmap24Image.bitmapFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);

    bitmap24Image.bitmapInfoHeaderV3.biWidth = width;
    bitmap24Image.bitmapInfoHeaderV3.biHeight = height;
    bitmap24Image.bitmapInfoHeaderV3.biSize = sizeof(BitmapInfoHeaderV3);
    bitmap24Image.bitmapInfoHeaderV3.biPlanes = 1;
    bitmap24Image.bitmapInfoHeaderV3.biBitCount = 24;
    bitmap24Image.bitmapInfoHeaderV3.biCompression = 0;
    bitmap24Image.bitmapInfoHeaderV3.biSizeImage = imageSize;
    bitmap24Image.bitmapInfoHeaderV3.biXPelsPerMeter = 2835;
    bitmap24Image.bitmapInfoHeaderV3.biYPelsPerMeter = 2835;
    bitmap24Image.bitmapInfoHeaderV3.biClrUsed = 0;
    bitmap24Image.bitmapInfoHeaderV3.biClrImportant = 0;

    return bitmap24Image;
}
