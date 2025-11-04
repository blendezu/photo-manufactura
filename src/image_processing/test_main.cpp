#include <iostream>
#include <opencv2/opencv.hpp>

#include "core/image_pipeline.h"
#include "light/brightness_adjust.h"
#include "operations/geometry/crop.h"
#include "operations/geometry/flip.h"
#include "operations/geometry/rotate.h"

int main() {
    std::cout << "Einfacher Pipeline Test: Flip → Rotate → Crop → Brightness" << std::endl;

    try {
        // 1. Einfaches Testbild erstellen
        cv::Mat testImage = cv::imread("/Users/duongtran/documents/testBilder/lenna.jpg");
        // cv::cvtColor(testImage, testImage, cv::COLOR_BGR2GRAY);
        if (testImage.empty()) {
            std::cerr << "❌ Bild konnte nicht geladen werden!" << std::endl;
            return 1;
        }
        // 2. Pipeline erstellen und Bild setzen
        ImagePipeline pipeline;
        pipeline.setImg(testImage);

        // 3. VERTIKAL SPIEGELN (Flip)
        std::cout << "1. Vertikal spiegeln..." << std::endl;
        pipeline.addOperation(std::make_shared<Flip>(1));  // 0 = vertikal

        // 4. ROTIEREN
        std::cout << "2. 45° rotieren..." << std::endl;

        pipeline.addOperation(
            std::make_shared<Rotate>(25, cv::Rect(0, 0, testImage.cols, testImage.rows)));

        // 5. ZUSCHNEIDEN (Crop)
        std::cout << "3. Zuschneiden..." << std::endl;
        // pipeline.addOperation(std::make_shared<Crop>(cv::Rect(00, 00, 2000, 1400)));

        // 6. Brightness
        pipeline.addOperation(std::make_shared<BrightnessAdjust>(80));

        // 7. FINALES ERGEBNIS ANZEIGEN
        std::cout << "Finales Ergebnis anzeigen..." << std::endl;
        cv::Mat result = pipeline.process();

        // Mit OpenCV anzeigen
        cv::imshow("Original Bild", testImage);
        cv::imshow("Bearbeitetes Bild", result);

        std::cout << "Drücke eine Taste zum Beenden..." << std::endl;
        cv::waitKey(0);

        std::cout << "✅ Test erfolgreich!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Fehler: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}