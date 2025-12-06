#include "ImageProcessingService.h"

#include <QDebug>

// Include the actual image processing headers
#include "../image_processing/core/image_pipeline.h"
#include "../image_processing/core/operation_registry.h"

// OpenCV for conversions
#include <opencv2/imgproc.hpp>

ImageProcessingService::ImageProcessingService(QObject* parent)
    : QObject(parent), m_pipeline(std::make_unique<ImagePipeline>()) {
    qDebug() << "ImageProcessingService created";
}

ImageProcessingService::~ImageProcessingService() {
    qDebug() << "ImageProcessingService destroyed";
}

// ============================================================================
// Image Management
// ============================================================================

bool ImageProcessingService::setImage(const QImage& image) {
    if (image.isNull()) {
        emit processingError("Cannot set null image");
        return false;
    }

    try {
        m_originalQImage = image;

        // Convert QImage to cv::Mat
        cv::Mat mat;
        QImage convertedImage = image.convertToFormat(QImage::Format_RGB888);

        mat = cv::Mat(convertedImage.height(), convertedImage.width(), CV_8UC3,
                      const_cast<uchar*>(convertedImage.bits()),
                      static_cast<size_t>(convertedImage.bytesPerLine()))
                  .clone();

        // OpenCV uses BGR, Qt uses RGB
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

        m_pipeline->setImg(mat);
        m_pipeline->clearOperations();

        emit pipelineChanged();
        emit undoRedoStateChanged(canUndo(), canRedo());

        qDebug() << "Image set successfully:" << image.width() << "x" << image.height();
        return true;

    } catch (const std::exception& e) {
        emit processingError(QString("Failed to set image: %1").arg(e.what()));
        return false;
    }
}

QImage ImageProcessingService::getOriginalImage() const {
    return m_originalQImage;
}

QImage ImageProcessingService::getProcessedImage() {
    if (!hasImage()) {
        return QImage();
    }

    try {
        cv::Mat result = m_pipeline->process();
        return matToQImage(&result);
    } catch (const std::exception& e) {
        emit processingError(QString("Processing failed: %1").arg(e.what()));
        return m_originalQImage;
    }
}

bool ImageProcessingService::hasImage() const {
    return m_pipeline->hasImg();
}

// ============================================================================
// Filter Operations
// ============================================================================

QStringList ImageProcessingService::getFiltersByCategory(const QString& category) const {
    OperationRegistry::Category cat;

    if (category.toLower() == "monochrome") {
        cat = OperationRegistry::Category::MONOCHROME;
    } else if (category.toLower() == "vintage") {
        cat = OperationRegistry::Category::VINTAGE;
    } else {
        cat = OperationRegistry::Category::GENERAL;
    }

    auto filters = OperationRegistry::getInstance().getFiltersByCategory(cat);
    QStringList result;
    for (const auto& filter : filters) {
        result.append(QString::fromStdString(filter));
    }
    return result;
}

bool ImageProcessingService::applyFilter(const QString& filterName) {
    if (!hasImage()) {
        emit processingError("No image loaded");
        return false;
    }

    try {
        auto operation = OperationRegistry::getInstance().createFilter(filterName.toStdString());

        if (!operation) {
            emit processingError(QString("Unknown filter: %1").arg(filterName));
            return false;
        }

        m_pipeline->addOperation(operation);

        emit pipelineChanged();
        emit undoRedoStateChanged(canUndo(), canRedo());

        // Process and emit result
        QImage result = getProcessedImage();
        emit processingComplete(result);

        return true;

    } catch (const std::exception& e) {
        emit processingError(QString("Failed to apply filter: %1").arg(e.what()));
        return false;
    }
}

// ============================================================================
// Adjustment Operations
// ============================================================================

bool ImageProcessingService::adjustBrightness(int value) {
    // TODO: Create brightness operation and add to pipeline
    // For now, emit a placeholder
    qDebug() << "Adjust brightness:" << value;
    return true;
}

bool ImageProcessingService::adjustContrast(int value) {
    qDebug() << "Adjust contrast:" << value;
    return true;
}

bool ImageProcessingService::adjustExposure(int value) {
    qDebug() << "Adjust exposure:" << value;
    return true;
}

bool ImageProcessingService::adjustHighlights(int value) {
    qDebug() << "Adjust highlights:" << value;
    return true;
}

bool ImageProcessingService::adjustShadows(int value) {
    qDebug() << "Adjust shadows:" << value;
    return true;
}

bool ImageProcessingService::adjustWhites(int value) {
    qDebug() << "Adjust whites:" << value;
    return true;
}

bool ImageProcessingService::adjustBlacks(int value) {
    qDebug() << "Adjust blacks:" << value;
    return true;
}

bool ImageProcessingService::adjustTemperature(int value) {
    qDebug() << "Adjust temperature:" << value;
    return true;
}

bool ImageProcessingService::adjustTint(int value) {
    qDebug() << "Adjust tint:" << value;
    return true;
}

bool ImageProcessingService::adjustSaturation(int value) {
    qDebug() << "Adjust saturation:" << value;
    return true;
}

bool ImageProcessingService::adjustVibrance(int value) {
    qDebug() << "Adjust vibrance:" << value;
    return true;
}

bool ImageProcessingService::adjustClarity(int value) {
    qDebug() << "Adjust clarity:" << value;
    return true;
}

bool ImageProcessingService::adjustSharpness(int value) {
    qDebug() << "Adjust sharpness:" << value;
    return true;
}

// ============================================================================
// Geometry Operations
// ============================================================================

bool ImageProcessingService::rotate(int degrees) {
    qDebug() << "Rotate:" << degrees << "degrees";
    return true;
}

bool ImageProcessingService::flip(bool horizontal) {
    qDebug() << "Flip:" << (horizontal ? "horizontal" : "vertical");
    return true;
}

bool ImageProcessingService::crop(int x, int y, int width, int height) {
    qDebug() << "Crop:" << x << y << width << height;
    return true;
}

// ============================================================================
// Live Preview
// ============================================================================

void ImageProcessingService::setLiveOperation(const QString& operationType, int value) {
    // TODO: Implement live preview with temporary operation
    Q_UNUSED(operationType);
    Q_UNUSED(value);
}

void ImageProcessingService::commitLiveOperation() {
    // TODO: Commit live operation to pipeline
}

void ImageProcessingService::cancelLiveOperation() {
    m_pipeline->clearLiveOperations();
}

QImage ImageProcessingService::getLivePreview() {
    // TODO: Process with live operation included
    return getProcessedImage();
}

// ============================================================================
// Undo/Redo
// ============================================================================

bool ImageProcessingService::canUndo() const {
    return m_pipeline->canUndo();
}

bool ImageProcessingService::canRedo() const {
    return m_pipeline->canRedo();
}

void ImageProcessingService::undo() {
    m_pipeline->undo();
    emit pipelineChanged();
    emit undoRedoStateChanged(canUndo(), canRedo());
    emit processingComplete(getProcessedImage());
}

void ImageProcessingService::redo() {
    m_pipeline->redo();
    emit pipelineChanged();
    emit undoRedoStateChanged(canUndo(), canRedo());
    emit processingComplete(getProcessedImage());
}

// ============================================================================
// Pipeline Management
// ============================================================================

void ImageProcessingService::resetToOriginal() {
    m_pipeline->clearOperations();
    m_pipeline->clearUndoHistory();
    emit pipelineChanged();
    emit undoRedoStateChanged(canUndo(), canRedo());
    emit processingComplete(getOriginalImage());
}

int ImageProcessingService::getOperationCount() const {
    return static_cast<int>(m_pipeline->getOperationCount());
}

QString ImageProcessingService::serializePipeline() const {
    return QString::fromStdString(m_pipeline->serializePipeline());
}

bool ImageProcessingService::deserializePipeline(const QString& data) {
    try {
        m_pipeline->deserialziePipeline(data.toStdString());
        emit pipelineChanged();
        return true;
    } catch (const std::exception& e) {
        emit processingError(QString("Failed to load pipeline: %1").arg(e.what()));
        return false;
    }
}

// ============================================================================
// Private Utilities
// ============================================================================

QImage ImageProcessingService::matToQImage(const void* matPtr) const {
    const cv::Mat& mat = *static_cast<const cv::Mat*>(matPtr);

    if (mat.empty()) {
        return QImage();
    }

    // Convert BGR to RGB
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);

    return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                  QImage::Format_RGB888)
        .copy();  // Deep copy to own the data
}

void* ImageProcessingService::qImageToMat(const QImage& image) const {
    // This returns a pointer to a stack-allocated cv::Mat, which is unsafe
    // In a real implementation, you'd need to manage memory properly
    // This is just a placeholder showing the conversion logic
    Q_UNUSED(image);
    return nullptr;
}

void ImageProcessingService::processAsync() {
    // TODO: Implement async processing with QThread or QtConcurrent
    // For now, processing is synchronous
}
