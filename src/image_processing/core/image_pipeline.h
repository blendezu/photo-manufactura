#ifndef IMAGE_PIPELINE_H
#define IMAGE_PIPELINE_H

#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include "operation_base.h"

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

    // --- Undo/Redo System (Command Pattern) ---
    struct PipelineAction {
        enum Type {
            INSERT, /**< Operation was inserted. Undo: Remove it. */
            REMOVE, /**< Operation was removed. Undo: Insert it back. */
            MODIFY  /**< Operation settings changed. Undo: Restore old settings/object. */
        };

        Type type;
        int index;                                    /**< Index where the action happened. */
        std::shared_ptr<ImageOperation> operation;    /**< The primary operation involved. */
        std::shared_ptr<ImageOperation> oldOperation; /**< For MODIFY: The previous state. */
    };

    std::vector<PipelineAction> m_undoStack;
    std::vector<PipelineAction> m_redoStack;

    // --- Cache State ---
    cv::Mat m_cachedResult; /** Cached image. */
    bool m_cacheValid;      /** True if the cache is valid. */
    std::shared_ptr<ImageOperation>
        m_liveOperation; /** Temporary operation for realtime sliders. */

    // --- Configuration --
    bool m_useFusedPipeline; /** If true, attempts to fuse Halide operations on the GPU */

   public:
    ImagePipeline();
    ~ImagePipeline() = default;

    // --- A. Image Management ---

    /**
     * @brief Loads a new image into the pipeline. Resets history and cache.
     * @param srcImg The image to load.
     */
    void setImg(const cv::Mat& srcImg);

    /**
     * @brief Returns a clone of the original image.
     */
    cv::Mat getOriginalImg() const {
        return m_originalImg.clone();
    }

    /**
     * @brief Checks if the pipeline has an image loaded.
     */
    bool hasImg() const {
        return !m_originalImg.empty();
    }

    // --- B. Operation Stack Management ---

    /**
     * @brief Adds an operation to the end of the pipeline.
     */
    void addOperation(std::shared_ptr<ImageOperation> operation);

    /**
     * @brief Adds an operation to any position in the pipeline.
     * @param index The index at which to insert the operation.
     * @param operation The operation to insert.
     */
    void insertOperation(int index, std::shared_ptr<ImageOperation>);

    /**
     * @brief Removes an operation at a specific index from the pipeline.
     * @param index The index of the operation to remove.
     */
    void removeOperation(int index);

    /**
     * @brief Modifies an existing operation at a given index.
     * Replaces the operation with a new instance (usually with new settings).
     * @param index The index of the operation to modify.
     * @param newOp The new operation state.
     */
    void modifyOperation(int index, std::shared_ptr<ImageOperation> newOp);

    /**
     * @brief Clears all operations from the pipeline.
     */
    void clearOperations();

    /**
     * @brief Clears the undo history.
     */
    void clearUndoHistory();

    /**
     * @brief Returns the number of operations in the pipeline.
     */
    size_t getOperationCount() const {
        return m_operations.size();
    }

    /**
     * @brief Returns the operation at the specified index.
     */
    std::shared_ptr<ImageOperation> getOperation(int index);

    /**
     * @brief Returns a reference to the list of operations.
     */
    const std::vector<std::shared_ptr<ImageOperation>>& getOperations() const {
        return m_operations;
    }

    // --- C. Live Preview ---

    /**
     * @brief Set an operation that is applied temporarily on top of the cached result.
     * Used for slider interactions to ensure high FPS
     */
    void setLiveOperation(std::shared_ptr<ImageOperation> operation);
    void clearLiveOperations();

    // --- D. Processing ---

    /**
     * @brief Executes the pipeline.
     * Decisions regarding caching and
     */
    cv::Mat process();
    cv::Mat processUpTo(int operationIndex);

    /**
     * @brief Toogles between Sequential and Fused mode
     * @param enabled True enables the Halide fusion engine.
     */
    void setFusionMode(bool enabled);

    /**
     * @brief Returns true if the pipeline is in fused mode.
     */
    bool isFusionMode() const {
        return m_useFusedPipeline;
    }

    /**
     * @brief Undo the last operation.
     * Takes the last operation from the pipeline and puts it back into the undone
     * operations stack.
     */
    void undo();

    /**
     * @brief Redo the last undone operation.
     * Takes the last operation from the undone operations stack (m_undoneOperations) and puts it
     * back into the pipeline (m_operations).
     */
    void redo();

    /**
     * @brief Checks if there are any operations to undo.
     */
    bool canUndo() const {
        return !m_undoStack.empty();
    }

    /**
     * @brief Checks if there are any operations to redo.
     */
    bool canRedo() const {
        return !m_redoStack.empty();
    }

    /**
     * @brief Returns the number of operations that can be undone.
     */
    size_t getUndoCount() const {
        return m_undoStack.size();
    }

    /**
     * @brief Invalidates the cache.
     */
    void invalidateCache();

    /**
     * @brief Checks if the cache is valid.
     */
    bool isCacheValid() const {
        return m_cacheValid;
    }

    /**
     * @brief Serializes the pipeline to a JSON string.
     * Use for saving the pipeline state as a preset.
     */
    std::string serializePipeline() const;

    /**
     * @brief Deserializes the pipeline from a JSON string.
     * Use for loading the pipeline state from a preset.
     */
    void deserializePipeline(const std::string& data);

   private:
    /**
     * @brief Updates the cache with a new result.
     */
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
    /**
     * @brief Cached pipeline for Halide operations.
     *
     */
    struct CachedPipeline {
        /**
         * @brief The key for the cache: Unique sequence of operations.
         * If the list of operations (or their order/parameters) changes, the cache is invalid.
         */
        std::vector<std::shared_ptr<HalideOperation>> ops;

        /**
         * @brief The compiled Halide program (The "Machine").
         * This object holds the JIT-compiled assembly code. Reusing this avoids
         * expensive recompilation (JIT overhead) when processing subsequent frames
         * with the same structure.
         */
        Halide::Pipeline pipeline;

        /**
         * @brief Abstract placeholder for the input image during compilation.
         * The JIT compiler needs to know "There will be an image here" without
         * knowing the exact pixel data yet.
         * During execution (.realize()), we plug the real cv::Mat into this socket.
         */
        Halide::ImageParam inputParam;

        /**
         * @brief Context validation: Input Bit Depth.
         * Initialized to -1 (Invalid) to force a compilation on the very first run.
         * If the input image changes from 8-bit to 16-bit, the compiled code
         * (which assumes specific data types) becomes invalid and must be recompiled.
         */
        int inputDepth = -1;

        /**
         * @brief Context validation: Number of Channels.
         * Initialized to -1 (Invalid).
         * If the input changes from RGB (3) to RGBA (4) or Grayscale (1),
         * the memory layout stride changes, requiring a recompile.
         */
        int inputChannels = -1;
    };

    CachedPipeline m_pipelineCache; /** Cached pipeline for Halide operations. */
};

#endif