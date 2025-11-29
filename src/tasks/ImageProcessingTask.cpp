#include "ImageProcessingTask.h"

#include <QDebug>
#include <opencv2/opencv.hpp>

// Include image processing components
#include "image_pipeline.h"
#include "operation_base.h"

ImageProcessingTask::ImageProcessingTask(
    int id, const cv::Mat& image, const std::vector<std::shared_ptr<ImageOperation>>& operations,
    TaskPriority priority, QObject* parent)
    : Task(id, priority, parent), m_image(image.clone()), m_operations(operations) {
    // Image is cloned for thread safety
    // Operations are copied (shared_ptr is thread-safe for read)
}

ImageProcessingTask::~ImageProcessingTask() {
    // Pipeline cleaned up automatically
}

void ImageProcessingTask::execute() {
    // Validate input
    if (m_image.empty()) {
        throw std::runtime_error("Input image is empty");
    }

    if (m_operations.empty()) {
        qDebug() << "No operations to apply, returning original image";
        setResult(QVariant::fromValue(m_image));
        return;
    }

    qDebug() << "Processing image:" << m_image.cols << "x" << m_image.rows << "with"
             << m_operations.size() << "operations";

    // Create isolated pipeline instance for this task
    m_pipeline = std::make_unique<ImagePipeline>();

    try {
        // Set source image
        m_pipeline->setImg(m_image);

        // Add all operations
        for (size_t i = 0; i < m_operations.size(); ++i) {
            // Check for cancellation before adding operation
            if (isCancelled()) {
                qDebug() << "Task cancelled during operation setup";
                return;  // emit cancelled() handled by Task::run()
            }

            m_pipeline->addOperation(m_operations[i]);

            // Report progress for operation addition (first 10%)
            int progressPercent = static_cast<int>((i + 1) * 10.0 / m_operations.size());
            setProgress(progressPercent);
        }

        // Check cancellation before heavy processing
        if (isCancelled()) {
            qDebug() << "Task cancelled before processing";
            return;
        }

        // Process the pipeline
        // Note: Individual operations should also check for cancellation
        // but ImagePipeline doesn't currently support this
        setProgress(15);  // Processing started

        cv::Mat result = m_pipeline->process();

        // Check cancellation after processing
        if (isCancelled()) {
            qDebug() << "Task cancelled after processing";
            return;
        }

        // Validate result
        if (result.empty()) {
            throw std::runtime_error("Pipeline processing returned empty image");
        }

        setProgress(100);  // Complete

        // Return result (will be emitted via completed signal)
        setResult(QVariant::fromValue(result));

        qDebug() << "Image processing completed successfully";

    } catch (const cv::Exception& e) {
        // OpenCV exception
        QString error = QString("OpenCV error: %1").arg(e.what());
        qWarning() << error;
        throw std::runtime_error(error.toStdString());

    } catch (const std::exception& e) {
        // Standard exception - rethrow to be caught by Task::run()
        qWarning() << "Image processing failed:" << e.what();
        throw;
    }
}
