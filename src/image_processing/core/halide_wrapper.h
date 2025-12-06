#ifndef HALIDE_WRAPPER_H
#define HALIDE_WRAPPER_H

#include <cstdlib>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <stdexcept>

#include "Halide.h"

/**
 * @brief Wrapper for Halide JIT compilation.
 * Implements Singleton pattern for Target detection to miniminze overhead.
 * Handles Debug/Release flags automatically.
 */
class HalideWrapper {
   public:
    enum class Backend { AUTO, CPU, CUDA, METAL, OPENCL };

   private:
    Halide::Target m_target;
    bool m_isGPU;

    // Static cache for the best detected target to avoid querying OS every time.
    static Halide::Target s_bestTarget;
    static bool s_initialized;
    static std::mutex s_initMutex;

   public:
    /**
     * @brief Constructor
     * @param backendForce Optional: Force a specific backend
     */
    HalideWrapper(Backend backendForce = Backend::AUTO);

    Halide::Target getTarget() const;
    bool isGPU() const;

    /**
     * @brief Wraps OpenCV Mat into Halide Buffer (Zero-Copy).
     */
    template <typename T>
    Halide::Buffer<T> wrap(const cv::Mat& mat) {
        if (mat.empty()) {
            throw std::runtime_error("[HalideWrapper] Error: Input image is empty.");
        }

        if (!mat.isContinuous()) {
            throw std::runtime_error(
                "[HalideWrapper] Error: cv::Mat is not continuous. Please clone() before calling!");
        }

        // check OpenCV dimension
        int w = mat.cols;
        int h = mat.rows;
        int c = mat.channels();

        // create buffer (Interleaved BGR BGR ...)
        return Halide::Buffer<T>::make_interleaved(reinterpret_cast<T*>(mat.data), w, h, c);
    }

    /**
     * @brief Applies scheduling
     * GPU: Tiling with boundary checks
     * CPU: Basic parallelization (Debug fallback only, because of using OpenMP).
     */
    void applySchedule(Halide::Func& f, Halide::Var x, Halide::Var y);

    std::string getTargetName() const;

   private:
    static Halide::Target detectBestTarget();
};

#endif  // HALIDE_WRAPPER_H