#include <iostream>
#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "color/saturation_adjust.h"
#include "color/tint_magenta.h"
#include "color/vibrance_adjust.h"
#include "color/white_balance.h"
#include "core/image_pipeline.h"
#include "light/black_adjust.h"
#include "light/brightness_adjust.h"
#include "light/contrast_adjust.h"
#include "light/highlight_adjust.h"
#include "light/shadow_adjust.h"
#include "light/white_adjust.h"
#include "operation_base.h"
#include "operations/effects/gray_image.h"
#include "operations/effects/vintage1.h"
#include "operations/geometry/crop.h"
#include "operations/geometry/flip.h"
#include "operations/geometry/rotate.h"
#include "utils/histogram.h"
#include "utils/image_resize.h"

int main() {
    // std::cout << "Einfacher Pipeline Test: Flip → Rotate → Crop → Brightness" << std::endl;

    try {
        // 1. Einfaches Testbild erstellen
        cv::Mat testImage = cv::imread("/Users/duongtran/documents/testBilder/baum.jpg");
        if (testImage.empty()) {
            std::cerr << "❌ Bild konnte nicht geladen werden!" << std::endl;
            return 1;
        }
        // cv::resize(testImage, testImage, cv::Size(testImage.cols / 6, testImage.rows / 6));
        // cv::cvtColor(testImage, testImage, cv::COLOR_BGR2GRAY);

        // histogram
        auto histImg = Histogram::histogramImg(testImage);
        cv::imshow("histOriginal", std::get<1>(histImg));

        // 2. Pipeline erstellen und Bild setzen
        ImagePipeline pipeline;
        pipeline.setImg(testImage);

        // 3. VERTIKAL SPIEGELN (Flip)
        // std::cout << "1. Vertikal spiegeln..." << std::endl;
        // pipeline.addOperation(std::make_shared<Flip>(1));  // 0 = vertikal

        // 4. ROTIEREN
        // std::cout << "2. 45° rotieren..." << std::endl;
        // pipeline.addOperation(
        //     std::make_shared<Rotate>(20, cv::Rect(0, 0, testImage.cols, testImage.rows)));

        // 5. ZUSCHNEIDEN (Crop)
        // std::cout << "3. Zuschneiden..." << std::endl;
        // pipeline.addOperation(std::make_shared<Crop>(cv::Rect(00, 00, 2000, 1400)));

        // 6. Brightness
        // pipeline.addOperation(std::make_shared<AdjustBrightness>(20));

        // 7. Contrast
        // pipeline.addOperation(std::make_shared<AdjustContrast>(100));

        // 8. Highlight
        // pipeline.addOperation(std::make_shared<AdjustHighlight>(-100));

        // 9. Shadow
        pipeline.addOperation(std::make_shared<AdjustShadow>(100));

        // 10. Saturation
        // pipeline.addOperation(std::make_shared<AdjustSaturation>(50));

        // 11. Vibrance
        // pipeline.addOperation(std::make_shared<AdjustVibrance>(100));

        // 12. White
        // pipeline.addOperation(std::make_shared<AdjustWhite>(100));

        // 12. Black
        // pipeline.addOperation(std::make_shared<AdjustBlack>(50));

        // 13. White balance
        // pipeline.addOperation(std::make_shared<WhiteBalance>(100));

        // 14. Tint Magenta
        // pipeline.addOperation(std::make_shared<TintMagenta>(100));

        // 15. Resize
        // pipeline.addOperation(std::make_shared<ResizeImage>(1000u, 1000u));

        // 16. Gray image
        // pipeline.addOperation(std::make_shared<GrayImage>());

        // 17. Vintage 1
        // pipeline.addOperation(std::make_shared<Vintage1>());

        // remove a image operation
        // pipeline.removeOperation(1);

        //  FINALES ERGEBNIS ANZEIGEN
        // std::cout << "Finales Ergebnis anzeigen..." << std::endl;
        cv::Mat result = pipeline.process();

        auto histNew = Histogram::histogramImg(result);
        cv::imshow("histNew", std::get<0>(histNew));

        // Mit OpenCV anzeigen
        cv::imshow("Original Bild", testImage);
        cv::imshow("Bearbeitetes Bild", result);

        // std::cout << "Drücke eine Taste zum Beenden..." << std::endl;
        cv::waitKey(0);

        std::cout << "✅ rotate erfolgreich!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Fehler: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}