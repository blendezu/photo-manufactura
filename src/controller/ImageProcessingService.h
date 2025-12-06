#pragma once

#include <QImage>
#include <QObject>
#include <QString>
#include <memory>

// Forward declarations - avoid including heavy OpenCV headers
class ImagePipeline;
class ImageOperation;

/**
 * @brief Service layer for image processing operations
 *
 * Provides a Qt-friendly interface to the ImagePipeline, handling:
 * - QImage <-> cv::Mat conversions
 * - Async processing with signals
 * - Filter discovery and application
 * - Undo/Redo management
 *
 * This service acts as a bridge between the Qt-based controller/UI
 * and the OpenCV-based image processing core.
 */
class ImageProcessingService : public QObject {
    Q_OBJECT

   public:
    explicit ImageProcessingService(QObject* parent = nullptr);
    ~ImageProcessingService();

    // Image management
    /**
     * @brief Set the source image for processing
     * @param image The input QImage
     * @return true if image was set successfully
     */
    bool setImage(const QImage& image);

    /**
     * @brief Get the original unprocessed image
     * @return Original QImage
     */
    QImage getOriginalImage() const;

    /**
     * @brief Get the currently processed image
     * @return Processed QImage with all operations applied
     */
    QImage getProcessedImage();

    /**
     * @brief Check if an image is loaded
     * @return true if image is loaded
     */
    bool hasImage() const;

    // Filter operations
    /**
     * @brief Get available filters by category
     * @param category Category name: "monochrome", "vintage", "general"
     * @return List of filter names
     */
    QStringList getFiltersByCategory(const QString& category) const;

    /**
     * @brief Apply a named filter
     * @param filterName Name of the filter to apply
     * @return true if filter was applied successfully
     */
    bool applyFilter(const QString& filterName);

    // Adjustment operations
    bool adjustBrightness(int value);   // -100 to 100
    bool adjustContrast(int value);     // -100 to 100
    bool adjustExposure(int value);     // -100 to 100
    bool adjustHighlights(int value);   // -100 to 100
    bool adjustShadows(int value);      // -100 to 100
    bool adjustWhites(int value);       // -100 to 100
    bool adjustBlacks(int value);       // -100 to 100
    bool adjustTemperature(int value);  // -100 to 100
    bool adjustTint(int value);         // -100 to 100
    bool adjustSaturation(int value);   // -100 to 100
    bool adjustVibrance(int value);     // -100 to 100
    bool adjustClarity(int value);      // -100 to 100
    bool adjustSharpness(int value);    // 0 to 100

    // Geometry operations
    bool rotate(int degrees);
    bool flip(bool horizontal);
    bool crop(int x, int y, int width, int height);

    // Live preview (for slider dragging)
    /**
     * @brief Set a temporary operation for live preview
     * The operation is not committed until commitLiveOperation() is called
     */
    void setLiveOperation(const QString& operationType, int value);

    /**
     * @brief Commit the current live operation to the pipeline
     */
    void commitLiveOperation();

    /**
     * @brief Cancel the current live operation
     */
    void cancelLiveOperation();

    /**
     * @brief Get preview with live operation applied
     * @return Image with live operation applied (not committed)
     */
    QImage getLivePreview();

    // Undo/Redo
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    // Pipeline management
    /**
     * @brief Clear all operations (reset to original)
     */
    void resetToOriginal();

    /**
     * @brief Get the number of operations in the pipeline
     */
    int getOperationCount() const;

    // Serialization
    QString serializePipeline() const;
    bool deserializePipeline(const QString& data);

   signals:
    /**
     * @brief Emitted when processing is complete
     * @param image The processed image
     */
    void processingComplete(const QImage& image);

    /**
     * @brief Emitted when an error occurs during processing
     * @param error Error message
     */
    void processingError(const QString& error);

    /**
     * @brief Emitted when undo/redo state changes
     */
    void undoRedoStateChanged(bool canUndo, bool canRedo);

    /**
     * @brief Emitted when pipeline changes
     */
    void pipelineChanged();

   private:
    // Conversion utilities
    QImage matToQImage(const void* mat) const;
    void* qImageToMat(const QImage& image) const;

    // Internal processing
    void processAsync();

    std::unique_ptr<ImagePipeline> m_pipeline;
    QImage m_originalQImage;  // Keep a copy for quick access
};
