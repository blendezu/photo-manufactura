#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

#include "raw_processing.h"

int main(int argc, char* argv[]) {
    try {
        // 16-bit RAW-Bild laden
        cv::Mat img16 =
            RawProcessing::loadRawImg("/Users/duongtran/Documents/testBilder/rawCanon2.cr3");

        if (img16.empty()) {
            std::cerr << "Fehler: Bild ist leer!" << std::endl;
            return -1;
        }

        std::cout << "16-bit RAW Bild: " << img16.cols << " x " << img16.rows << " (Typ: CV_16UC3)"
                  << std::endl;

        cv::Mat dstImg = img16.clone();

        for (int y = 0; y < img16.rows; y++) {
            const cv::Vec3w* img16Ptr = img16.ptr<cv::Vec3w>(y);
            cv::Vec3w* dstPtr = dstImg.ptr<cv::Vec3w>(y);

            for (int x = 0; x < img16.cols; x++) {
                int B = img16Ptr[x][0];
                int G = img16Ptr[x][1];
                int R = img16Ptr[x][2];

                dstPtr[x] = cv::Vec3w(B, G, R);
            }
        }

        // 16-bit zu 8-bit konvertieren
        cv::Mat img8;

        // Methode 1: Einfache Skalierung (0-65535 -> 0-255)
        dstImg.convertTo(img8, CV_8UC3, 1.0 / 256.0);

        std::cout << "Konvertiert zu 8-bit: " << img8.cols << " x " << img8.rows
                  << " (Typ: CV_8UC3)" << std::endl;

        // Bild anzeigen
        cv::imshow("RAW Bild - 16-bit zu 8-bit konvertiert", img8);

        std::cout << "Drücken Sie eine Taste um das Fenster zu schließen..." << std::endl;
        cv::waitKey(0);

    } catch (const std::exception& e) {
        std::cerr << "Fehler: " << e.what() << std::endl;
        return -1;
    }

    std::cout << "Test abgeschlossen." << std::endl;

    return 0;
}
