#pragma once

#include <memory>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "Task.h"

// Forward declaration
class ImageOperation;
class ImagePipeline;

/**
 * @brief Task for processing images through an operation pipeline
 *
 * Executes a sequence of ImageOperations on a source image
 * asynchronously. Designed to work with the ImagePipeline class.
 *
 * Thread Safety:
 * - Clones input image (safe to modify original after submission)
 * - Creates isolated ImagePipeline instance per task
 * - Result returned via signal to main thread
 */
class ImageProcessingTask : public Task {
    Q_OBJECT

   public:
    /**
     * @brief Construct an image processing task
     * @param id Task ID
     * @param image Source image to process (will be cloned)
     * @param operations List of operations to apply (in order)
     * @param priority Task priority
     * @param parent Parent QObject
     */
    explicit ImageProcessingTask(int id, const cv::Mat& image,
                                 const std::vector<std::shared_ptr<ImageOperation>>& operations,
                                 TaskPriority priority = TaskPriority::NORMAL,
                                 QObject* parent = nullptr);

    ~ImageProcessingTask() override;

    QString name() const override {
        return QString("ImageProcessingTask #%1 (%2 ops)").arg(id()).arg(m_operations.size());
    }

   protected:
    /**
     * @brief Execute image processing pipeline
     *
     * Process flow:
     * 1. Clone source image (thread safety)
     * 2. Create ImagePipeline instance
     * 3. Add all operations to pipeline
     * 4. Process image (with cancellation checks)
     * 5. Return result via setResult()
     *
     * Emits progress signals during processing.
     * Checks isCancelled() between operations.
     */
    void execute() override;

   private:
    cv::Mat m_image;
    std::vector<std::shared_ptr<ImageOperation>> m_operations;
    std::unique_ptr<ImagePipeline> m_pipeline;
};
