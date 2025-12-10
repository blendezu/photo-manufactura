#include <iostream>
#include <memory>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "color/saturation_adjust.h"
#include "color/tint_magenta.h"
#include "color/vibrance_adjust.h"
#include "color/white_balance.h"
#include "core/image_pipeline.h"
#include "image_utils.h"
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
#include "raw_processing.h"
#include "style_transfer.h"
#include "utils/color_space.h"
#include "utils/histogram.h"
#include "utils/image_resize.h"

int main() {
    try {
        // Bild laden
        // cv::Mat testImage = cv::imread("images/Baum.jpg");

        cv::Mat testImage = RawProcessing::loadRawImg("images/rawCanon.cr3");

        if (testImage.empty()) {
            std::cerr << "❌ Bild konnte nicht geladen werden!" << std::endl;
            return 1;
        }

        // Histogram vom Orignalbild
        auto oriHist = Histogram::histogramImg(testImage);

        // Bild für Anzeige vorbereiten (falls 16-bit)
        cv::Mat displayImage = testImage.clone();
        if (displayImage.depth() == CV_16U) {
            displayImage.convertTo(displayImage, CV_8UC3, 1.0 / 256.0);
        }

        // Pipeline erstellen
        ImagePipeline pipeline;
        pipeline.setFusionMode(false);

        pipeline.setImg(testImage);

        // Zähler für die Schritte
        int stepCounter = 0;

        // Hauptloop
        while (true) {
            cv::Mat currentResult;

            // Pipeline verarbeiten und Ergebnis für Anzeige vorbereiten

            cv::Mat processed = pipeline.process();
            pipeline.invalidateCache();

            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < 10; i++) {
                processed = pipeline.process();
                pipeline.invalidateCache();
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Time for processing: " << duration.count() / 10 << std::endl;

            currentResult = processed.clone();

            if (currentResult.depth() == CV_16U) {
                currentResult.convertTo(currentResult, CV_8UC3, 1.0 / 256.0);
            }

            auto currHist = Histogram::histogramImg(currentResult);

            cv::imshow("currentResult", currentResult);

            // Auf Tastendruck warten
            int key = cv::waitKey(0);

            if (key == 27) {  // ESC-Taste
                break;
            } else if (key == 110) {  // Taste n
                stepCounter++;

                // Erste Operation: Bild auf 1000x1000 reduzieren
                if (stepCounter == 1) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Resize-Operation\n";
                    pipeline.addOperation(std::make_shared<AdjustWhite>(-100));
                    // pipeline.addOperation(std::make_shared<AdjustShadow>(-100));
                }

                // Zweite Operation: Ändern des Werts von -100 auf 50 (Modify)
                else if (stepCounter == 2) {
                    std::cout << std::endl;
                    std::cout << std::endl;

                    std::cout << "➡️  Modify-Operation: White -100 -> 50" << std::endl;
                    pipeline.modifyOperation(0, std::make_shared<AdjustWhite>(50));
                }

                // Dritte Operation: Helligkeit erhöhen
                else if (stepCounter == 3) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Brightness-Operation hinzugefügt (+50)" << std::endl;
                    pipeline.addOperation(std::make_shared<AdjustBrightness>(50));
                    pipeline.addOperation(std::make_shared<AdjustContrast>(50));
                    pipeline.addOperation(std::make_shared<AdjustSaturation>(50));
                    pipeline.addOperation(std::make_shared<AdjustWhite>(50));
                    pipeline.addOperation(std::make_shared<AdjustBlack>(50));
                    pipeline.addOperation(std::make_shared<AdjustHighlight>(50));
                }

                // Dritte Operation: Helligkeit erhöhen
                else if (stepCounter == 4) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Histogram" << std::endl;
                    cv::imshow("current histogram", std::get<0>(currHist));
                    cv::imshow("original histogram", std::get<0>(oriHist));

                }

                // Vierte Operation: Undo Helligkeit für Graubild
                else if (stepCounter == 5) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Undo Brightness-Operation" << std::endl;
                    cv::destroyWindow("current histogram");
                    cv::destroyWindow("original histogram");
                    pipeline.undo();
                }

                // 5. Operation: Graubild
                else if (stepCounter == 6) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Graubild umwandeln" << std::endl;
                    pipeline.addOperation(std::make_shared<GrayImage>());
                }

                // 6. Operation: Helligkeit vom Graubild reduzieren
                else if (stepCounter == 7) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Brightness -50" << std::endl;
                    pipeline.addOperation(std::make_shared<AdjustBrightness>(-50));
                }

                // 7. Operation: Histogram anzeigen
                else if (stepCounter == 8) {
                    std::cout << std::endl;
                    std::cout << std::endl;
                    std::cout << "➡️  Histogram" << std::endl;
                    cv::imshow("current histogram", std::get<0>(currHist));
                }

                // 8. Operation: das bearbeitete Bild abspeichern
                else if (stepCounter == 9) {
                    cv::destroyWindow("current histogram");
                    cv::destroyWindow("original histogram");
                    cv::imwrite("newImg.jpg", processed);
                    std::cout << "➡️ das Bild abgespeichert\n";
                }

                else {
                    std::cout << "ℹ️  Alle Schritte abgeschlossen" << std::endl;
                    stepCounter = 10;
                }

            } else if (key == 98) {  // Taste b (Undo)
                if (pipeline.canUndo()) {
                    pipeline.undo();
                    stepCounter = std::max(0, stepCounter - 1);
                    cv::destroyWindow("current histogram");
                    cv::destroyWindow("original histogram");
                    std::cout << "↩️  Undo ausgeführt. Schritt: " << stepCounter
                              << ", Operationen: " << pipeline.getOperationCount()
                              << ", Undo verfügbar: " << pipeline.getUndoCount() << std::endl;
                } else {
                    std::cout << "❌ Keine Operation zum Rückgängig machen verfügbar." << std::endl;
                }
            }
        }

        std::cout << "✅ Programm erfolgreich beendet!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Fehler: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}