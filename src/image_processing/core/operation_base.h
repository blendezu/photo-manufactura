#ifndef OPERATION_BASE_H
#define OPERATION_BASE_H

#include <opencv2/core/hal/interface.h>

#include <exception>
#include <opencv2/opencv.hpp>
#include <string>

#include "Halide.h"
#include "halide_wrapper.h"

/**
 * @brief Abstract base class for all image processing operations.
 */
class ImageOperation {
   public:
    virtual ~ImageOperation() = default;

    virtual bool supportsHalide() const {
        return false;
    }

    // Check for Statistic Calculation
    virtual bool requiresFreshStats() const {
        return false;
    }

    // Apply this function on the image (CPU)
    virtual cv::Mat apply(const cv::Mat& srcImg) = 0;

    // For Fused Engine
    virtual Halide::Func applyHalide(Halide::Func input, [[maybe_unused]] Halide::Var x,
                                     [[maybe_unused]] Halide::Var y,
                                     [[maybe_unused]] Halide::Var c) {
        return input;
    }

    // Name for the function on the GUI
    virtual std::string getName() const = 0;

    // Get settings (for save/load)
    virtual std::string getSettings() const {
        return "";
    }

    // Calculate output dimensions based on input dimensions.
    // Used for Crop, because Halide thinks the input size is always identical to output size
    // Mainly overwritten when crop image, set another values for dstWidth and dstHeight.
    // If output and input have the same size, dont need to be implemented, tandard is destination
    // size = source size
    virtual void getOutputDimensions(int srcWidth, int srcHeight, int& dstWidth,
                                     int& dstHeight) const {
        dstWidth = srcWidth;
        dstHeight = srcHeight;
    }
};

/**
 * @brief Base class for GPU-accelerated operations using Halide
 */
class HalideOperation : public ImageOperation {
   public:
    bool supportsHalide() const override {
        return true;
    }

    /**
     * @brief Constructs the Halide computation graph
     * Pure virtual: Must be implemented by subclasses.
     */
    virtual Halide::Func buildGraph(Halide::Func input, Halide::Var x, Halide::Var y,
                                    Halide::Var c) = 0;

    /**
     * @brief Calculates parameters in CPU before GPU
     * Standard: doing nothing
     */
    virtual void prepareParameters([[maybe_unused]] const cv::Mat& srcImg) {}

    // Implementation for fusion interface
    Halide::Func applyHalide(Halide::Func input, Halide::Var x, Halide::Var y,
                             Halide::Var c) override {
        return buildGraph(input, x, y, c);
    }

    // Implementation for single execution (Legacy/Fallback)
    // This can be used for protying. The new Operation can run on CPU only with buildGraph
    // implemented.
    // But mostly this function will be overwritten.
    cv::Mat apply(const cv::Mat& srcImg) override {
        if (srcImg.empty())
            return srcImg;

        int depth = srcImg.depth();

        if (depth == CV_8U) {
            return runJit<uint8_t>(srcImg, 255.0f);
        } else if (depth == CV_16U) {
            return runJit<uint16_t>(srcImg, 65535.0f);
        } else {
            std::cerr
                << "[HalideOperation] Error: unsupported depth. Only 8-bit or 16-bit supported\n";
            return srcImg;
        }
    }

   private:
    /**
     * @brief Helper to run the JIT compiler for a single operation.
     */
    template <typename T>
    cv::Mat runJit(const cv::Mat& srcImg, float maxValue) {
        HalideWrapper hw;

        try {
            // Calculate parameters (CPU)
            prepareParameters(srcImg);

            Halide::Var x("x"), y("y"), c("c");

            // Zero-Copy Wrap (input)
            Halide::Buffer<T> inputBuf = hw.wrap<T>(srcImg);

            // cast to Float for precise math
            Halide::Func inputFunc("in_func");
            inputFunc(x, y, c) = inputBuf(x, y, c);

            // call the logic
            Halide::Func processed = buildGraph(inputFunc, x, y, c);

            // cast back + clamp
            Halide::Func outputFunc("out_func");
            outputFunc(x, y, c) =
                Halide::cast<T>(Halide::clamp(processed(x, y, c), 0.0f, maxValue));

            // optimize
            hw.applySchedule(outputFunc, x, y);

            // create the result image
            int dstW, dstH;
            getOutputDimensions(srcImg.cols, srcImg.rows, dstW, dstH);
            cv::Mat dstImg(dstH, dstW, srcImg.type());
            Halide::Buffer<T> outputBuf = hw.wrap<T>(dstImg);

            // compile and run
            outputFunc.realize(outputBuf, hw.getTarget());

            if (hw.isGPU()) {
                outputBuf.copy_to_host();
                outputBuf.device_sync();
            }

            return dstImg;
        } catch (Halide::Error& e) {
            std::cerr << "[HalideOperation] JIT Error: " << e.what() << std::endl;
            return srcImg;
        } catch (std::exception& e) {
            std::cerr << "[HalideOperation] Std Error: " << e.what() << std::endl;
            return srcImg;
        }
    }
};

#endif