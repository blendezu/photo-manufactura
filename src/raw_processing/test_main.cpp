#include <iostream>
#include <opencv2/opencv.hpp>

#include "raw_processing.h"

int main(int argc, char* argv[]) {
    try {
        RawProcessing rawP;

        // 16-bit RAW-Bild laden
        cv::Mat img16 = rawP.getRawImg("/Users/duongtran/Documents/testBilder/rawCanon.cr3");

        if (img16.empty()) {
            std::cerr << "Fehler: Bild ist leer!" << std::endl;
            return -1;
        }

        std::cout << "16-bit RAW Bild: " << img16.cols << " x " << img16.rows << " (Typ: CV_16UC3)"
                  << std::endl;

        // 16-bit zu 8-bit konvertieren
        cv::Mat img8;

        // Methode 1: Einfache Skalierung (0-65535 -> 0-255)
        img16.convertTo(img8, CV_8UC3, 1.0 / 256.0);

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