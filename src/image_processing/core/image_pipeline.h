#ifndef IMAGE_PIPELINE_H
#define IMAGE_PIPELINE_H

#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include "../core/operation_base.h"

/**
 * @class ImagePipeline
 * @brief Manages the image processing pipeline.
 * * This class holds the original image an a list of operations.
 * It supports:
 * - Sequential CPU processing (legacy/stable mode).
 * - Experimental Fused GPU processing via Halide.
 * - Dual-Engine execution: Switching between sequential CPU processing and GPU fusion via Halide
 * - Undo/Redo history.
 * - Result caching to optimize live previews.
 */
class ImagePipeline {
   private:
    cv::Mat m_originalImg;                                     /**< The immutable source image. */
    std::vector<std::shared_ptr<ImageOperation>> m_operations; /**< List of active operations. */
    std::vector<std::shared_ptr<ImageOperation>> m_undoneOperations; /**< Stack for Undo/Redo. */

    // --- Cache State ---
    cv::Mat m_cachedResult; /** Cached image. */
    bool m_cacheValid;      /**  */
    std::shared_ptr<ImageOperation>
        m_liveOperation; /** Temporary operation for realtime sliders. */

    // --- Configuration --
    bool m_useFusedPipeline; /**< If true, attempts to fuse Halide operations on the GPU */

   public:
    ImagePipeline();
    ~ImagePipeline() = default;

    // --- Image Management ---

    /**
     * @brief Loads a new image into the pipeline. Resets history and cache.
     * @param srcImg
     */
    void setImg(const cv::Mat& srcImg);

    cv::Mat getOriginalImg() const {
        return m_originalImg.clone();
    }
    bool hasImg() const {
        return !m_originalImg.empty();
    }

    // --- Operation Stack Management ---

    // add an operation to the end of the pipeline
    void addOperation(std::shared_ptr<ImageOperation> operation);

    // add an operation to any position
    void insertOperation(int index, std::shared_ptr<ImageOperation>);

    // When a slider set to 0 --> remove that operation
    void removeOperation(int index);

    // mainly for the Reset button (undo still works)
    void clearOperations();

    void clearUndoHistory();

    // call the operation
    size_t getOperationCount() const {
        return m_operations.size();
    }
    std::shared_ptr<ImageOperation> getOperation(int index);
    const std::vector<std::shared_ptr<ImageOperation>>& getOperations() const {
        return m_operations;
    }

    // --- Live Preview ---

    /**
     * @brief Set an operation that is applied temporarily on top of the cached result.
     * Used for slider interactions to ensure high FPS
     */
    void setLiveOperation(std::shared_ptr<ImageOperation> operation);
    void clearLiveOperations();

    // --- Processing ---

    /**
     * @brief Executes the pipeline.
     * Decisions regarding caching and
     */
    cv::Mat process();
    cv::Mat processUpTo(int operationIndex);

    /**
     * @brief Toogles between Sequential and Fused mode
     * @param enabled True enables the Halide fustion engine.
     */
    void setFusionMode(bool enabled);
    bool isFusionMode() const {
        return m_useFusedPipeline;
    }

    // Undo/Redo
    void undo();
    void redo();
    bool canUndo() const {
        return !m_operations.empty();
    }
    bool canRedo() const {
        return !m_undoneOperations.empty();
    }
    size_t getUndoCount() const {
        return m_undoneOperations.size();
    }

    // Cache management
    void invalidateCache();
    bool isCacheValid() const {
        return m_cacheValid;
    }

    std::string serializePipeline() const;
    void deserializePipeline(const std::string& data);

   private:
    void updateCache(const cv::Mat& result);

    /**
     * @brief Legacy Engine: Excecutes operations sequentially on CPU.
     */
    cv::Mat processSequential();

    /**
     * @brief GPU Engine: Excecutes opertaions on GPU
     * Use fallback to CPU for unsupported filters.
     */
    cv::Mat processFused();

    /**
     * @brief Helper to execute a batch of Halide operations
     */
    cv::Mat runFusedHalideChain(const cv::Mat& srcImg,
                                std::vector<std::shared_ptr<HalideOperation>>& ops);

    // --- JIT Caching ---
    struct CachedPipeline {
        std::vector<std::shared_ptr<HalideOperation>> ops;
        Halide::Pipeline pipeline;
        Halide::ImageParam inputParam;
        int inputDepth = -1;
        int inputChannels = -1;
    };

    CachedPipeline m_pipelineCache;
};

#endif