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
            }

            else if (key == 110) {  // Taste n

                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << "➡️  Resize-Operation\n";
                // Zeile 120-136: ANGEPASST an ImageController-Reihenfolge + AOT-Generator
                // WICHTIG: Denoise wird im JIT nicht separat aufgerufen (nur in AOT als
                // Halide-Filter)

                // 1. RGB Block
                pipeline.addOperation(std::make_shared<AdjustExposure>(-0.5));  // 1. Exposure
                pipeline.addOperation(std::make_shared<WhiteBalance>(20));      // 2. Temperature
                pipeline.addOperation(std::make_shared<TintMagenta>(20));       // 3. TintMagenta

                // 2. HSL Block
                pipeline.addOperation(std::make_shared<AdjustBrightness>(20));  // 4. Brightness
                pipeline.addOperation(std::make_shared<AdjustHighlight>(20));   // 5. Highlight
                pipeline.addOperation(std::make_shared<AdjustShadow>(20));      // 6. Shadow
                pipeline.addOperation(std::make_shared<AdjustWhite>(20));       // 7. White
                pipeline.addOperation(std::make_shared<AdjustBlack>(20));       // 8. Black
                pipeline.addOperation(std::make_shared<AdjustContrast>(20));    // 9. Contrast
                pipeline.addOperation(std::make_shared<AdjustSaturation>(20));  // 10. Saturation
                pipeline.addOperation(std::make_shared<AdjustVibrance>(20));    // 11. Vibrance

                // 3. Spatial Block
                pipeline.addOperation(std::make_shared<Sharpen>(20));  // 12. Sharpen
                pipeline.addOperation(std::make_shared<Clarity>(20));  // 13. Clarity

                // Geometry Operations (must match AOT test for fair comparison)
                // cv::Rect cropRoi = cv::Rect(0, 0, testImage.cols, testImage.rows);
                // pipeline.addOperation(std::make_shared<Crop>(cropRoi));  // 13. Crop

                // cv::Rect roi2 = cv::Rect(0, 0, testImage.cols, testImage.rows);
                // pipeline.addOperation(std::make_shared<Rotate>(10, roi2));  // 14. Rotate
                // pipeline.addOperation(std::make_shared<Flip>(1));           // 15. Flip
            }

            else if (key == 98) {  // Taste B
                pipeline.clearOperations();
                std::cout << "Clear all opearations.\n" << std::endl;

            } else if (key == 97) {  // Taste 'a' (AOT Test)
                std::cout << "🚀 Running AOT Pipeline..." << std::endl;

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
                // 1. RGB
                state.exposure = -0.5f;
                state.temp = 20.0f;
                state.tintMagenta = 20.0f;

                // 2. HSL
                state.brightness = 20.0f;
                state.highlight = 20.0f;
                state.shadow = 20.0f;
                state.white = 20.0f;
                state.black = 20.0f;
                state.contrast = 20.0f;
                state.saturation = 20.0f;
                state.vibrance = 20.0f;

                // 3. Spatial
                state.sharpen = 20.0f;
                state.clarity = 20.0f;

                // 4. Geometry
                // state.flip = 1;

                // Test fast path (denoise = 0) vs slow path (denoise > 0)
                // state.denoise = 0.0f;
                // state.resizeHeight = 3000.0f;
                // state.resizeWidth = 3000.0f;

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