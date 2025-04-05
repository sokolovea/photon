#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <memory>
#include <eigen3/Eigen/Dense>
#include <vector>
#include <cmath>
#include "bitmap.h"
#include "bitmap.h"
#include "t_matrix.h"

using namespace std;

enum class ZoneClaudePotier8 {
    One = 0,
    Two,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Unreacheable//Third
};

enum class ZoneClaudePotier16 {
    OneOne = 0,
    OneTwo,
    TwoOne,
    TwoTwo,
    FourOne,
    FourTwo,
    FiveOne,
    FiveTwo,
    SixOne,
    SixTwo,
    SevenOne,
    SevenTwo,
    EightOne,
    EightTwo,
    NineOne,
    NineTwo,
    Unreacheable
};


Bitmap24Pixel getPixelByZone(ZoneClaudePotier8 zone, bool isUncontrolled = false) {
    switch (zone) {
        case ZoneClaudePotier8::One:
            return Bitmap24Pixel(176, 203, 10);
        case ZoneClaudePotier8::Two:
            return Bitmap24Pixel(110, 183, 77);
        case ZoneClaudePotier8::Four:
            return Bitmap24Pixel(255, 236, 0);
        case ZoneClaudePotier8::Five:
            return Bitmap24Pixel(29, 167, 66);
        case ZoneClaudePotier8::Six:
            if (isUncontrolled) {
                return Bitmap24Pixel(67,109,180);
            }
            return Bitmap24Pixel(185, 121, 82);
        case ZoneClaudePotier8::Seven:
            return Bitmap24Pixel(230, 52, 19);
        case ZoneClaudePotier8::Eight:
            return Bitmap24Pixel(39, 104, 34);
        case ZoneClaudePotier8::Nine:
            return Bitmap24Pixel(13, 82, 160);
        default:
            return Bitmap24Pixel(0, 0, 0);
    }
}


Bitmap24Pixel getPixelByZone(ZoneClaudePotier16 zone) {
    switch (zone) {
        case ZoneClaudePotier16::OneOne:
            return Bitmap24Pixel(176, 203, 10);
        case ZoneClaudePotier16::OneTwo:
            return Bitmap24Pixel(136, 169, 24);
        case ZoneClaudePotier16::TwoOne:
            return Bitmap24Pixel(110, 183, 77);
        case ZoneClaudePotier16::TwoTwo:
            return Bitmap24Pixel(98, 196, 220);
        case ZoneClaudePotier16::FourOne:
            return Bitmap24Pixel(255, 236, 0);
        case ZoneClaudePotier16::FourTwo:
            return Bitmap24Pixel(209, 195, 0);
        case ZoneClaudePotier16::FiveOne:
            return Bitmap24Pixel(29, 167, 66);
        case ZoneClaudePotier16::FiveTwo:
            return Bitmap24Pixel(21, 132, 45);
        case ZoneClaudePotier16::SixOne:
            return Bitmap24Pixel(186, 121, 82);
        case ZoneClaudePotier16::SixTwo:
            return Bitmap24Pixel(67, 109, 180);
        case ZoneClaudePotier16::SevenOne:
            return Bitmap24Pixel(230, 52, 19);
        case ZoneClaudePotier16::SevenTwo:
            return Bitmap24Pixel(183, 35, 14);
        case ZoneClaudePotier16::EightOne:
            return Bitmap24Pixel(39, 104, 34);
        case ZoneClaudePotier16::EightTwo:
            return Bitmap24Pixel(22, 71, 21);
        case ZoneClaudePotier16::NineOne:
            return Bitmap24Pixel(13, 82, 160);
        case ZoneClaudePotier16::NineTwo:
            return Bitmap24Pixel(33, 58, 143);
        default:
            return Bitmap24Pixel(0, 0, 0);
    }
}

ZoneClaudePotier8 classificateClaudePotierToZone(const Bitmap24Pixel& processingPixel) {
    double alpha = processingPixel.red * 90 / 255.0;
    double h = processingPixel.green / 255.0;
    if (h > 0.9) {
        if (alpha > 55) {
            return ZoneClaudePotier8::One;
        }
        if (alpha > 40) {
            return ZoneClaudePotier8::Two;
        }
    } else if (h > 0.5) {
        if (alpha > 50) {
            return ZoneClaudePotier8::Four;
        }
        if (alpha > 40) {
            return ZoneClaudePotier8::Five;
        }
        return ZoneClaudePotier8::Six;
    } else {
        if (alpha > 47.5) {
            return ZoneClaudePotier8::Seven;
        }
        if (alpha > 42.5) {
            return ZoneClaudePotier8::Eight;
        }
        return ZoneClaudePotier8::Nine;
    }
    // // 3 (unreachable)
    return ZoneClaudePotier8::Unreacheable;
}


ZoneClaudePotier16 classificateClaudePotierExtendedToZone(const Bitmap24Pixel& processingPixel) {
    double alpha = processingPixel.red * 90 / 255.0;
    double h = processingPixel.green / 255.0;
    double A = processingPixel.blue / 255.0;
    if (A <= 0.5) {
        if (h > 0.9) {
            if (alpha > 55) {
                return ZoneClaudePotier16::OneOne;
            }
            if (alpha > 40) {
                return ZoneClaudePotier16::TwoOne;
            }
        } else if (h > 0.5) {
            if (alpha > 50) {
                return ZoneClaudePotier16::FourOne;
            }
            if (alpha > 40) {
                return ZoneClaudePotier16::FiveOne;
            }
            return ZoneClaudePotier16::SixOne;
        } else {
            if (alpha > 47.5) {
                return ZoneClaudePotier16::SevenOne;
            }
            if (alpha > 42.5) {
                return ZoneClaudePotier16::EightOne;
            }
            return ZoneClaudePotier16::NineOne;
        }
    } else {
        if (h > 0.9) {
            if (alpha > 55) {
                // cout << alpha << "; " << h << "; " << A << endl;
                return ZoneClaudePotier16::OneTwo;
            }
            if (alpha > 40) {
                return ZoneClaudePotier16::TwoTwo;
            }
        } else if (h > 0.5) {
            if (alpha > 50) {
                return ZoneClaudePotier16::FourTwo;
            }
            if (alpha > 40) {
                return ZoneClaudePotier16::FiveTwo;
            }
            return ZoneClaudePotier16::SixTwo;
        } else {
            if (alpha > 47.5) {
                return ZoneClaudePotier16::SevenTwo;
            }
            if (alpha > 42.5) {
                return ZoneClaudePotier16::EightTwo;
            }
            return ZoneClaudePotier16::NineTwo;
        }
    }
    // 3 (unreachable)
    return ZoneClaudePotier16::Unreacheable;
}


Eigen::Matrix3cd calculateAverageMatrix(const vector<Eigen::Matrix3cd>& pixels, vector<int64_t>& indexes) {
    Eigen::Matrix3cd average = Eigen::Matrix3cd::Zero();
    for (const auto& index : indexes) {
        average = average + pixels[index];
    }
    if (!indexes.empty()) {
        average = average / indexes.size();
    }
    return average;
}

std::complex<double> wishartDistance(const Eigen::Matrix3cd& T, const Eigen::Matrix3cd& T_ref) {
    Eigen::Matrix3cd T_ref_inv = T_ref.inverse();
    std::complex<double> trace = (T_ref_inv * T).trace();
    return std::log(T_ref.determinant()) + trace;
}

void reclassify(vector<Eigen::Matrix3cd>& pixels, vector<Eigen::Matrix3cd>& T_avg, vector<int>& classifications, int numClasses, double percent) {
    int64_t total = pixels.size();
    const int64_t threshold = pixels.size() * percent / 100;

    while (total > threshold) {
        total = 0;
        vector<vector<int64_t>> indexMatrices(numClasses);
        vector<Eigen::Matrix3cd> newT_avg(numClasses, Eigen::Matrix3cd::Zero());

        cout << "New Iteration!" << endl;

        // Параллелизация цикла по пикселям (можно использовать OpenMP)
        #pragma omp parallel for
        for (size_t i = 0; i < pixels.size(); ++i) {
            Eigen::Matrix3cd& T = pixels[i];

            std::complex<double> minDistance = std::numeric_limits<double>::max();
            int bestClass = -1;

            for (int c = 0; c < numClasses - 1; c++) { // Исправлено: используем numClasses
                auto distance = wishartDistance(T, T_avg[c]);
                if (std::abs(distance) < std::abs(minDistance)) { // Исправлено: сравнение модуля
                    minDistance = distance;
                    bestClass = c;
                }
            }

            #pragma omp critical
            {
                if (classifications[i] != bestClass) {
                    classifications[i] = bestClass;
                    total++;
                }

                if (bestClass >= 0) {
                    indexMatrices[bestClass].push_back(i);
                }
            }
        }

        // Обновление средних значений матриц
        #pragma omp parallel for
        for (int c = 0; c < numClasses - 1; ++c) {
            if (!indexMatrices[c].empty()) {
                newT_avg[c] = calculateAverageMatrix(pixels, indexMatrices[c]);
            } /*else {
                newT_avg[c] = T_avg[c]; // Сохраняем старое значение, если класс пуст
            }*/
        }
        T_avg.swap(newT_avg);

        // Подсчет количества пикселей в каждом классе
        vector<int> classCounts(numClasses, 0);
        for (int cls : classifications) {
            classCounts[cls]++;
        }
        for (size_t i = 0; i < classCounts.size(); ++i) {
            cout << "Class " << i + 1 << ": " << classCounts[i] << " pixels" << endl;
        }
        cout << "Total pixel changes = " << total << ": " << total * 100.0 / pixels.size() << " %" << endl;
    }
}


enum class WorkMode {
    Classificate8,
    Classificate16,
    ClassificateWishart8,
    ClassificateWishart16
};

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Wrong parameters count!" << endl;
        return 1;
    }

    WorkMode workMode;
    bool isUncontrolled = false;
    string mode = argv[1];
    if (mode == "classificate8") {
        workMode = WorkMode::Classificate8;
    } else if (mode == "classificate16") {
        workMode = WorkMode::Classificate16;
    } else if (mode == "classificateWishart8") {
        isUncontrolled = true;
        workMode = WorkMode::ClassificateWishart8;
    } else if (mode == "classificateWishart16") {
        isUncontrolled = true;
        workMode = WorkMode::ClassificateWishart16;
    } else {
        cerr << "Unknown work mode!" << endl;
        return 1;
    }

    ifstream inputStream;
    inputStream.open(argv[2], ios_base::binary);

    ofstream outputStream;
    outputStream.open(argv[3], ios_base::binary);

    if (!inputStream.is_open() || !outputStream.is_open()) {
        cerr << "Can't open input files!" << endl;
        return 2;
    }

    double percent;
    ifstream rawFile;
    if (workMode == WorkMode::ClassificateWishart8 ||
        workMode == WorkMode::ClassificateWishart16) {
        if (argc < 6) {
            cerr << "Lack of parameters!" << endl;
        }
        rawFile.open(argv[4], ios::binary);
        if (!rawFile.is_open()) {
            cerr << "Can't open raw file with T!" << endl;
            return 3;
        }
        try {
            percent = std::stod(argv[5]);
            if (percent < 0.0) {
                std::cout << "Value can't be less 0!" << std::endl;
                return 3;
            }
        } catch (...) {
            std::cerr << "Invalid percent argument: not a number!" << std::endl;
        }
    }

    BitmapFileHeader bitmapInputFileHeader;
    BitmapInfoHeaderV3 bitmapInputInfoHeader;

    inputStream.read((char*)&bitmapInputFileHeader, sizeof(BitmapFileHeader));
    if (bitmapInputFileHeader.bfType != 0x4d42 || inputStream.fail()) {
        cerr << "Not BMP file!" << endl;
        return 3;
    }

    inputStream.read((char*)&bitmapInputInfoHeader, sizeof(BitmapInfoHeaderV3));
    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);

    if (inputStream.fail()) {
        cerr << "Error reading source file!" << endl;
        return 4;
    }

    if (bitmapInputInfoHeader.biBitCount != 24) {
        cerr << "Not 24 bit BMP file!" << endl;
        return 5;
    }

    int32_t inputWidthPx = bitmapInputInfoHeader.biWidth;
    int32_t inputHeightPx = bitmapInputInfoHeader.biHeight;

    int32_t outputWidthPx = inputWidthPx;
    int32_t outputHeight = inputHeightPx;

    BitmapFileHeader bitmapOutputFileHeader = bitmapInputFileHeader;
    BitmapInfoHeaderV3 bitmapOutputInfoHeader = bitmapInputInfoHeader;

    uint64_t inputRowBytesCountWithoutPadding = inputWidthPx * 3;
    uint64_t outputRowBytesCountWithoutPadding = outputWidthPx * 3;

    uint64_t inputRowBytesCountWithPadding = getRowSizeWithPadding(inputRowBytesCountWithoutPadding);
    uint64_t outputRowBytesCountWithPadding = getRowSizeWithPadding(outputRowBytesCountWithoutPadding);

    uint64_t paddingBytes = inputRowBytesCountWithPadding - inputRowBytesCountWithoutPadding;

    std::unique_ptr<const Bitmap24Pixel[]> inputRow = std::make_unique<Bitmap24Pixel[]>(inputWidthPx);
    std::unique_ptr<Bitmap24Pixel[]> outputRow = std::make_unique<Bitmap24Pixel[]>(outputWidthPx + 1);

    bitmapOutputFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + outputRowBytesCountWithPadding * outputHeight;
    bitmapOutputFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);
    outputStream.write((char*)&bitmapOutputFileHeader, sizeof(BitmapFileHeader));
    outputStream.write((char*)&bitmapOutputInfoHeader, sizeof(BitmapInfoHeaderV3));

    bitmapOutputInfoHeader.biWidth = outputWidthPx;
    bitmapOutputInfoHeader.biHeight = outputHeight;
    std::vector<Eigen::Matrix3cd> tMatrix(inputWidthPx * inputHeightPx); //rawFile
    vector<int> classifications(inputWidthPx * inputHeightPx);


    std::vector<TMatrix> rowTMatrix(inputWidthPx); //from rawFile

    if (workMode == WorkMode::ClassificateWishart8 || workMode == WorkMode::ClassificateWishart16) {
        for (int32_t i = 0; i < inputHeightPx; i++) {
            rawFile.read((char*)rowTMatrix.data(), inputWidthPx * sizeof(TMatrix));
            if (rawFile.fail()) {
                cerr << "Error reading raw file!" << endl;
                return 7;
            }
            for (int32_t j = 0; j < inputWidthPx; j++) {
                tMatrix[(inputHeightPx - i - 1) * inputWidthPx + j] = rowTMatrix[j].getEigenMatrix();
            }
        }
    }

    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);

    for (int32_t i = 0; i < inputHeightPx; i++) {
        inputStream.read((char*)inputRow.get(), inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes, ios_base::cur);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
        }
        for (int32_t j = 0; j < inputWidthPx; j++) {
            if (workMode == WorkMode::ClassificateWishart16 || workMode == WorkMode::Classificate16) {
                classifications[i * inputWidthPx + j] = static_cast<int>(classificateClaudePotierExtendedToZone((inputRow[j])));
            } else {
                classifications[i * inputWidthPx + j] = static_cast<int>(classificateClaudePotierToZone((inputRow[j])));
            }
        }
    }

    int64_t classesCount = 9;
    if (workMode == WorkMode::Classificate16 || workMode == WorkMode::ClassificateWishart16) {
        classesCount = 17;
    }

    inputStream.seekg(bitmapInputFileHeader.bfOffBits, ios_base::beg);

    vector<Eigen::Matrix3cd> T_avg(classesCount, Eigen::Matrix3cd::Zero());
    vector<int64_t> dividers(classesCount, 0);
    for (int64_t i = 0; i < tMatrix.size(); i++) {
        T_avg[classifications[i]] += tMatrix[i];
        dividers[classifications[i]] += 1;
    }
    for (int64_t i = 0; i < T_avg.size(); i++) {
        T_avg[i] /= dividers[i];
    }

    vector<int> classCounts(classesCount, 0);
    for (int cls : classifications) {
        classCounts[cls]++;
    }
    for (size_t i = 0; i < classCounts.size(); ++i) {
        cout << "Class " << i + 1 << ": " << classCounts[i] << " pixels" << endl;
    }
    cout << "Reclassify" << endl;

    if (workMode == WorkMode::ClassificateWishart8 || workMode == WorkMode::ClassificateWishart16) {
        reclassify(tMatrix, T_avg, classifications, classesCount, percent);
    }

    for (int32_t i = 0; i < inputHeightPx; i++) {
        inputStream.read((char*)inputRow.get(), inputRowBytesCountWithoutPadding);
        inputStream.seekg(paddingBytes, ios_base::cur);
        if (inputStream.fail()) {
            cerr << "Error reading source image!" << endl;
        }
        for (int32_t j = 0; j < inputWidthPx; j++) {
            int pixelIndex = i * inputWidthPx + j;
            int pixelClass = classifications[pixelIndex];
            if (workMode == WorkMode::Classificate8 || workMode == WorkMode::ClassificateWishart8) {
                outputRow[j] = getPixelByZone(static_cast<ZoneClaudePotier8>(pixelClass), isUncontrolled);
            } else {
                outputRow[j] = getPixelByZone(static_cast<ZoneClaudePotier16>(pixelClass));
            }
        }
        outputStream.write((char*)outputRow.get(), outputRowBytesCountWithPadding);
    }

    outputStream.close();
    inputStream.close();
    cout << "Success!" << endl;
    return 0;
}
