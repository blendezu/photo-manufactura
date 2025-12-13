#include <chrono>
#include <iostream>
#include <memory>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "color/saturation_adjust.h"
#include "color/tint_magenta.h"
#include "color/vibrance_adjust.h"
#include "color/white_balance.h"
#include "color_space.h"
#include "core/image_pipeline.h"
#include "detail/clarity.h"
#include "detail/sharpen.h"
#include "effects/gray_image.h"
#include "effects/vintage1.h"
#include "geometry/crop.h"
#include "geometry/flip.h"
#include "geometry/rotate.h"
#include "histogram.h"
#include "image_controller.h"
#include "image_resize.h"
#include "image_utils.h"
#include "light/black_adjust.h"
#include "light/brightness_adjust.h"
#include "light/contrast_adjust.h"
#include "light/exposure_adjust.h"
#include "light/highlight_adjust.h"
#include "light/shadow_adjust.h"
#include "light/white_adjust.h"
#include "operation_base.h"
#include "raw_processing.h"
#include "style_transfer.h"

int main() {
    try {
        // Bild laden
        // cv::Mat testImage = cv::imread("images/Baum.jpg");

        cv::Mat testImage = RawProcessing::loadRawImg("images/rawCanon.cr3");

        // cv::cvtColor(testImage, testImage, cv::COLOR_BGR2GRAY);

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

            // --- COLD START - JIT COMPILATION ---
            auto cold_start = std::chrono::high_resolution_clock::now();

            cv::Mat processed = pipeline.process();

            auto cold_end = std::chrono::high_resolution_clock::now();
            auto cold_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(cold_end - cold_start);
            std::cout << "Time for Cold Start: " << cold_duration.count() << std::endl;
            pipeline.invalidateCache();

            // --- NO JIT COMPLATION - LIKE AOT -
            const int ITERATIONS = 1;
            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < ITERATIONS; i++) {
                processed = pipeline.process();
                pipeline.invalidateCache();
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Time for processing: " << duration.count() << std::endl;

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
                    // Zeile 120-136: ANGEPASST an ImageController-Reihenfolge + AOT-Generator
                    // WICHTIG: Denoise wird im JIT nicht separat aufgerufen (nur in AOT als
                    // Halide-Filter)

                    pipeline.addOperation(std::make_shared<AdjustExposure>(
                        0));  // 0. Exposure (im Generator immer aktiv)
                    pipeline.addOperation(std::make_shared<WhiteBalance>(50));  // 1. Temperature
                    pipeline.addOperation(
                        std::make_shared<AdjustBrightness>(100));  // 2. Brightness
                    pipeline.addOperation(std::make_shared<AdjustHighlight>(-100));  // 3. Highlight
                    pipeline.addOperation(std::make_shared<AdjustShadow>(20));       // 4. Shadow
                    pipeline.addOperation(std::make_shared<AdjustWhite>(-100));      // 5. White
                    pipeline.addOperation(std::make_shared<AdjustBlack>(100));       // 6. Black
                    pipeline.addOperation(std::make_shared<AdjustContrast>(100));    // 7. Contrast
                    pipeline.addOperation(
                        std::make_shared<AdjustSaturation>(-100));                // 8. Saturation
                    pipeline.addOperation(std::make_shared<AdjustVibrance>(50));  // 9. Vibrance
                    pipeline.addOperation(std::make_shared<TintMagenta>(50));     // 10. TintMagenta
                    // Denoise wird hier NICHT hinzugefügt (existiert nicht als separate
                    // JIT-Operation)
                    pipeline.addOperation(std::make_shared<Sharpen>(0));  // 11. Sharpen
                    pipeline.addOperation(std::make_shared<Clarity>(0));  // 12. Clarity

                    // Geometry Operations (must match AOT test for fair comparison)
                    cv::Rect cropRoi = cv::Rect(0, 0, testImage.cols, testImage.rows);
                    pipeline.addOperation(std::make_shared<Crop>(cropRoi));  // 13. Crop

                    cv::Rect roi2 = cv::Rect(0, 0, testImage.cols, testImage.rows);
                    pipeline.addOperation(std::make_shared<Rotate>(10, roi2));  // 14. Rotate
                    pipeline.addOperation(std::make_shared<Flip>(1));           // 15. Flip
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
            } else if (key == 97) {  // Taste 'a' (AOT Test)
                std::cout << "🚀 Running AOT Pipeline..." << std::endl;

                // 1. Prepare Input Buffer
                // Make sure we have a clean buffer.
                // Halide expects interleaved by default if not specified otherwise, but Generator
                // usually handles HWC. photo_adjustment generator defined Input<Buffer<uint8_t>>
                // input{"input", 3}; -> HWC assumption.

                cv::Mat inputMat = currentResult.clone();
                // Ensure 3 channels
                if (inputMat.channels() != 3) {
                    cv::cvtColor(inputMat, inputMat, cv::COLOR_GRAY2BGR);
                }

                // 4. AOT Pipeline über Controller testen (Wie GUI)
                std::cout << "🚀 Running AOT Pipeline via Controller..." << std::endl;

                ImageController controller;
                controller.setImage(currentResult);  // oder inputMat?

                // State (Slider-Werte) setzen
                ImageState state;
                state.exposure = 0.0f;  // 0. Exposure (neutral)
                state.white = -100.0f;
                state.shadow = 20.0f;
                state.highlight = -100.0f;
                state.contrast = 100.0f;

                state.black = 100.0f;
                state.brightness = 100.0f;  // Controller AOT Logic ignores this to prevent washout!

                state.saturation = -100.0f;
                state.tintMagenta = 50.0f;
                state.vibrance = 50.0f;
                state.temp = 50.0f;
                // state.flip = 1.0f;

                // state.rotation = 10.0f;
                // state.cropRect = cv::Rect(0, 0, currentResult.cols, currentResult.rows);

                auto aot_start = std::chrono::high_resolution_clock::now();

                controller.update(state);
                cv::Mat aotResult = controller.process();

                auto aot_end = std::chrono::high_resolution_clock::now();
                auto aot_duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(aot_end - aot_start);

                if (!aotResult.empty()) {
                    std::cout << "✅ AOT Pipeline Success! Time: " << aot_duration.count() << " ms"
                              << std::endl;
                    cv::imshow("AOT Result", aotResult);
                } else {
                    std::cerr << "❌ AOT Pipeline Failed (Empty Result)" << std::endl;
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