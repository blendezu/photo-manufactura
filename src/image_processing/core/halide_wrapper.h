#ifndef HALIDE_WRAPPER_H
#define HALIDE_WRAPPER_H

#include <cstdlib>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <stdexcept>

#include "Halide.h"

/**
 * @brief Wrapper for Halide JIT compilation acting as a bridge between OpenCV and Halide.
 *
 * This class handles three main responsibilities:
 * 1. Hardware Management: Automatically detects and caches the optimal compute target
 * (CPU/GPU/Metal/CUDA) using a Singleton pattern.
 * 2. Data Translation: Provides zero-copy wrapping of OpenCV implementations (cv::Mat) into Halide
 * Buffers.
 * 3. Scheduling Optimization: Applies context-aware scheduling strategies (e.g., tiling for GPU,
 * parallelization for CPU) to abstract complexity.
 */
class HalideWrapper {
   public:
    // Backend enum
    enum class Backend { AUTO, CPU, CUDA, METAL, OPENCL };

   private:
    Halide::Target m_target; /**< Target used by Halide */
    bool m_isGPU;            /**< Whether the target is a GPU */

    // Static cache for the best detected target to avoid querying OS every time.
    static Halide::Target s_bestTarget; /**< Static cache for the best detected target */
    static bool s_initialized;          /**< Whether the wrapper is initialized */
    static std::mutex s_initMutex;      /**< Mutex for thread-safe initialization */

   public:
    /**
     * @brief Constructor that initializes the Halide context.
     *
     * @param backendForce Optional: Forces a specific compute backend.
     * - If set to Backend::AUTO (default), it detects and uses the best available hardware
     * (cached).
     * - If set to explicit backend (e.g. CPU, METAL), it ignores the cache and forces that specific
     * target.
     */
    HalideWrapper(Backend backendForce = Backend::AUTO);

    /**
     * @brief Returns the current target used by Halide.
     */
    Halide::Target getTarget() const;

    /**
     * @brief Returns whether the current target is a GPU.
     */
    bool isGPU() const;

    /**
     * @brief Creates a Halide Buffer that points to the existing OpenCV memory (Zero-Copy).
     *
     * Uses `make_interleaved` to handle the pixel layout (Pixel interleaving: R G B R G B...)
     * without duplicating data.
     *
     * @warning The returned buffer shares ownership of the data pointer.
     *          Do not destroy existing cv::Mat while using this buffer!
     *
     * @param mat The OpenCV Mat to wrap.
     * @return Halide::Buffer<T> A view into the OpenCV memory.
     * @throw std::runtime_error If the input image is empty or non-continuous in memory.
     */
    template <typename T>
    Halide::Buffer<T> wrap(const cv::Mat& mat) {
        // 1. Check if the input image is empty
        if (mat.empty()) {
            throw std::runtime_error("[HalideWrapper] Error: Input image is empty.");
        }

        // 2. Check if the input image is continuous
        if (!mat.isContinuous()) {
            throw std::runtime_error(
                "[HalideWrapper] Error: cv::Mat is not continuous. Please use OpenCV clone() "
                "before calling!");
        }

        // 3. Get OpenCV dimension
        int w = mat.cols;
        int h = mat.rows;
        int c = mat.channels();

        // Dimensions 2: channels; Stride (to the next channel) 1
        auto buf = Halide::Buffer<T>::make_interleaved(reinterpret_cast<T*>(mat.data), w, h, c);
        // Debug
        // std::cout << "[HalideWrapper] Wrapped Buffer: " << w << "x" << h << "x" << c 
        //           << " min=" << buf.dim(0).min() << "," << buf.dim(1).min() << "," << buf.dim(2).min()
        //           << " stride=" << buf.dim(0).stride() << "," << buf.dim(1).stride() << "," << buf.dim(2).stride() << std::endl;
        return buf;
    }

    /**
     * @brief Applies scheduling for Halide Func.
     *
     * @param f The Halide Func to schedule.
     * @param x The x variable of the Func.
     * @param y The y variable of the Func.
     *
     * @throw std::runtime_error If the input image is empty or not continuous.
     */
    void applySchedule(Halide::Func& f, Halide::Var x, Halide::Var y);

    /**
     * @brief Returns the name of the current target used by Halide.
     */
    std::string getTargetName() const;

   private:
    /**
     * @brief Detects the best target for Halide based on the system.
     */
    static Halide::Target detectBestTarget();
};

#endif  // HALIDE_WRAPPER_H