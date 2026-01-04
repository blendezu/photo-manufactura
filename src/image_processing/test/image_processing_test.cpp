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
#include "denoise/denoise.h"
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
#include "light/auto_light.h"
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
        // Create synthetic image (500x500 green)
        cv::Mat testImage(500, 500, CV_8UC3, cv::Scalar(0, 255, 0));
        
        std::cout << "✅ Created synthetic test image: " << testImage.cols << "x" << testImage.rows << std::endl;

        // Pipeline erstellen
        ImagePipeline pipeline;
        // Explicitly ensure Fusion is FALSE (though default is now false)
        pipeline.setFusionMode(false);
        pipeline.setImg(testImage);
        std::cout << "✅ Image set to pipeline. Fusion Mode: " << (pipeline.isFusionMode() ? "ON" : "OFF") << std::endl;

        // Add operations (that previously crashed on GPU)
        pipeline.addOperation(std::make_shared<AdjustExposure>(1.0));
        pipeline.addOperation(std::make_shared<AdjustContrast>(20));
        pipeline.addOperation(std::make_shared<Sharpen>(50));

        std::cout << "✅ Operations added. Processing..." << std::endl;

        // Process
        auto start = std::chrono::high_resolution_clock::now();
        cv::Mat result = pipeline.process();
        auto end = std::chrono::high_resolution_clock::now();
        
        if (result.empty()) {
             std::cerr << "❌ Result is empty!" << std::endl;
             return 1;
        }

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "✅ Processing complete in " << duration.count() << " ms" << std::endl;
        std::cout << "✅ Smoke test passed!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Fehler: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}