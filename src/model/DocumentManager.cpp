#include "DocumentManager.h"

#include <QDebug>
#include <QFileInfo>
#include <QImage>

// ImagePipeline and operations from image_processing component
// Note: include paths are relative to image_processing's PUBLIC include directories
// These headers include Halide.h internally, but QT_NO_KEYWORDS is defined
// which prevents Qt's emit/signals/slots from conflicting with Halide's emit
#include "color/saturation_adjust.h"
#include "color/tint_magenta.h"
#include "color/white_balance.h"
#include "geometry/crop.h"
#include "geometry/flip.h"
#include "geometry/rotate.h"
#include "image_pipeline.h"
#include "light/black_adjust.h"
#include "light/brightness_adjust.h"
#include "light/contrast_adjust.h"
#include "light/highlight_adjust.h"
#include "light/shadow_adjust.h"
#include "light/white_adjust.h"

namespace {
// Helper: Convert QImage to cv::Mat
cv::Mat qImageToCvMat(const QImage& image) {
    QImage converted = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(converted.height(), converted.width(), CV_8UC3,
                const_cast<uchar*>(converted.bits()),
                static_cast<size_t>(converted.bytesPerLine()));
    cv::Mat result;
    cv::cvtColor(mat.clone(), result, cv::COLOR_RGB2BGR);
    return result;
}

// Helper: Convert cv::Mat to QImage
QImage cvMatToQImage(const cv::Mat& mat) {
    if (mat.empty())
        return QImage();

    cv::Mat rgb;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, rgb, cv::COLOR_GRAY2RGB);
    } else {
        rgb = mat;
    }

    // Handle different bit depths
    cv::Mat normalized;
    if (rgb.depth() == CV_16U) {
        rgb.convertTo(normalized, CV_8UC3, 1.0 / 256.0);
    } else {
        normalized = rgb;
    }

    return QImage(normalized.data, normalized.cols, normalized.rows,
                  static_cast<int>(normalized.step), QImage::Format_RGB888)
        .copy();
}
}  // namespace

DocumentManager::DocumentManager(QObject* parent)
    : QObject(parent),
      m_currentDocument(std::make_unique<ImageDocument>(this)),
      m_adjustments(std::make_unique<AdjustmentSettings>(this)),
      m_imagePipeline(std::make_unique<ImagePipeline>()) {
    // Connect adjustment changes to document modified state
    connect(m_adjustments.get(), &AdjustmentSettings::anySettingChanged, this, [this]() {
        if (m_currentDocument && !m_currentDocument->originalImage().isNull()) {
            m_currentDocument->setModified(true);
            applyAdjustments();
        }
    });

    // Forward document state changes
    connect(m_currentDocument.get(), &ImageDocument::modifiedChanged, this,
            &DocumentManager::documentStateChanged);
}

DocumentManager::~DocumentManager() = default;

bool DocumentManager::hasDocument() const {
    return m_currentDocument && !m_currentDocument->originalImage().isNull();
}

bool DocumentManager::hasUnsavedChanges() const {
    return m_currentDocument && m_currentDocument->isModified();
}

QString DocumentManager::currentFilePath() const {
    return m_currentDocument ? m_currentDocument->filePath() : QString();
}

QString DocumentManager::currentFileName() const {
    return m_currentDocument ? m_currentDocument->fileName() : QString();
}

bool DocumentManager::openDocument(const QString& filePath) {
    if (filePath.isEmpty()) {
        Q_EMIT errorOccurred("No file path provided");
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        Q_EMIT errorOccurred(QString("File does not exist: %1").arg(filePath));
        return false;
    }

    QImage image(filePath);
    if (image.isNull()) {
        Q_EMIT errorOccurred(QString("Failed to load image: %1").arg(filePath));
        return false;
    }

    // Clear any previous state
    m_adjustments->resetAll();
    m_currentDocument->clear();
    m_imagePipeline->clearOperations();

    // Set up new document
    m_currentDocument->setFilePath(filePath);
    m_currentDocument->setFormat(fileInfo.suffix().toLower());
    m_currentDocument->setOriginalImage(image);
    m_currentDocument->setProcessedImage(image);  // Start with unmodified
    m_currentDocument->setModified(false);

    // Set the image in the pipeline for processing
    cv::Mat cvImage = qImageToCvMat(image);
    m_imagePipeline->setImg(cvImage);
    qDebug() << "Image loaded:" << image.width() << "x" << image.height();

    Q_EMIT documentOpened(filePath);
    Q_EMIT documentStateChanged();
    return true;
}

bool DocumentManager::saveDocument() {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to save");
        return false;
    }

    QString filePath = m_currentDocument->filePath();
    if (filePath.isEmpty()) {
        Q_EMIT errorOccurred("No file path set - use Save As");
        return false;
    }

    return saveDocumentAs(filePath);
}

bool DocumentManager::saveDocumentAs(const QString& filePath) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to save");
        return false;
    }

    if (filePath.isEmpty()) {
        Q_EMIT errorOccurred("No file path provided");
        return false;
    }

    const QImage& imageToSave = m_currentDocument->processedImage();
    if (imageToSave.isNull()) {
        Q_EMIT errorOccurred("No image data to save");
        return false;
    }

    if (!imageToSave.save(filePath)) {
        Q_EMIT errorOccurred(QString("Failed to save image: %1").arg(filePath));
        return false;
    }

    // Update document state
    m_currentDocument->setFilePath(filePath);
    QFileInfo fileInfo(filePath);
    m_currentDocument->setFormat(fileInfo.suffix().toLower());
    m_currentDocument->setModified(false);

    Q_EMIT documentSaved(filePath);
    Q_EMIT documentStateChanged();
    return true;
}

void DocumentManager::closeDocument() {
    m_adjustments->resetAll();
    m_currentDocument->clear();
    m_imagePipeline->clearOperations();

    Q_EMIT documentClosed();
    Q_EMIT documentStateChanged();
}

void DocumentManager::newDocument(int width, int height) {
    // Create a blank white image
    QImage blankImage(width, height, QImage::Format_RGB32);
    blankImage.fill(Qt::white);

    m_adjustments->resetAll();
    m_currentDocument->clear();
    m_currentDocument->setOriginalImage(blankImage);
    m_currentDocument->setProcessedImage(blankImage);
    m_currentDocument->setModified(false);

    Q_EMIT documentCreated();
    Q_EMIT documentStateChanged();
}

void DocumentManager::applyAdjustments() {
    if (!hasDocument()) {
        return;
    }

    if (!m_imagePipeline->hasImg()) {
        qDebug() << "No image in pipeline, reloading from document";
        cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());
        m_imagePipeline->setImg(cvImage);
    }

    // Clear existing operations and rebuild based on current adjustment settings
    m_imagePipeline->clearOperations();

    // Build a combined operation for real-time preview using liveOperation
    // For now, we add individual operations for each non-zero adjustment

    // Light operations
    if (m_adjustments->brightness() != 0) {
        m_imagePipeline->addOperation(
            std::make_shared<AdjustBrightness>(m_adjustments->brightness()));
    }

    if (m_adjustments->contrast() != 0) {
        m_imagePipeline->addOperation(std::make_shared<AdjustContrast>(m_adjustments->contrast()));
    }

    if (m_adjustments->highlights() != 0) {
        m_imagePipeline->addOperation(
            std::make_shared<AdjustHighlight>(m_adjustments->highlights()));
    }

    if (m_adjustments->shadows() != 0) {
        m_imagePipeline->addOperation(std::make_shared<AdjustShadow>(m_adjustments->shadows()));
    }

    if (m_adjustments->whites() != 0) {
        m_imagePipeline->addOperation(std::make_shared<AdjustWhite>(m_adjustments->whites()));
    }

    if (m_adjustments->blacks() != 0) {
        m_imagePipeline->addOperation(std::make_shared<AdjustBlack>(m_adjustments->blacks()));
    }

    // Color operations
    if (m_adjustments->temperature() != 0) {
        m_imagePipeline->addOperation(std::make_shared<WhiteBalance>(m_adjustments->temperature()));
    }

    if (m_adjustments->tint() != 0) {
        m_imagePipeline->addOperation(std::make_shared<TintMagenta>(m_adjustments->tint()));
    }

    if (m_adjustments->saturation() != 0) {
        m_imagePipeline->addOperation(
            std::make_shared<AdjustSaturation>(m_adjustments->saturation()));
    }

    // Process the pipeline
    cv::Mat result = m_imagePipeline->process();

    if (!result.empty()) {
        QImage processedQImage = cvMatToQImage(result);
        m_currentDocument->setProcessedImage(processedQImage);
        qDebug() << "Image processed with" << m_imagePipeline->getOperationCount() << "operations";
    } else {
        qDebug() << "Pipeline processing returned empty result, keeping original";
        m_currentDocument->setProcessedImage(m_currentDocument->originalImage());
    }
}

void DocumentManager::rotateImage(int degrees) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to rotate");
        return;
    }

    // Get the current image (processed if available, otherwise original)
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->processedImage());

    // Apply rotation
    auto rotateOp = std::make_shared<Rotate>(degrees);
    cv::Mat rotated = rotateOp->apply(cvImage);

    if (!rotated.empty()) {
        QImage rotatedQImage = cvMatToQImage(rotated);

        // Update both original and processed - this is a destructive operation
        m_currentDocument->setOriginalImage(rotatedQImage);
        m_currentDocument->setProcessedImage(rotatedQImage);

        // Update the pipeline with the new base image
        m_imagePipeline->setImg(rotated);

        m_currentDocument->setModified(true);
        Q_EMIT imageTransformed();
        qDebug() << "Image rotated by" << degrees << "degrees";
    } else {
        Q_EMIT errorOccurred("Failed to rotate image");
    }
}

void DocumentManager::flipImage(int direction) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to flip");
        return;
    }

    // Get the current image
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->processedImage());

    // Apply flip (0 = vertical, 1 = horizontal)
    auto flipOp = std::make_shared<Flip>(direction);
    cv::Mat flipped = flipOp->apply(cvImage);

    if (!flipped.empty()) {
        QImage flippedQImage = cvMatToQImage(flipped);

        // Update both original and processed - this is a destructive operation
        m_currentDocument->setOriginalImage(flippedQImage);
        m_currentDocument->setProcessedImage(flippedQImage);

        // Update the pipeline with the new base image
        m_imagePipeline->setImg(flipped);

        m_currentDocument->setModified(true);
        Q_EMIT imageTransformed();
        qDebug() << "Image flipped" << (direction == 1 ? "horizontally" : "vertically");
    } else {
        Q_EMIT errorOccurred("Failed to flip image");
    }
}

void DocumentManager::cropImage(const QRect& cropArea) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to crop");
        return;
    }

    if (!cropArea.isValid() || cropArea.isEmpty()) {
        Q_EMIT errorOccurred("Invalid crop area");
        return;
    }

    // Get the current image
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->processedImage());

    // Validate crop area against image dimensions
    cv::Rect cvCropRect(cropArea.x(), cropArea.y(), cropArea.width(), cropArea.height());
    if (cvCropRect.x < 0 || cvCropRect.y < 0 || cvCropRect.x + cvCropRect.width > cvImage.cols ||
        cvCropRect.y + cvCropRect.height > cvImage.rows) {
        Q_EMIT errorOccurred("Crop area exceeds image bounds");
        return;
    }

    // Apply crop
    auto cropOp = std::make_shared<Crop>(cvCropRect);
    cv::Mat cropped = cropOp->apply(cvImage);

    if (!cropped.empty()) {
        QImage croppedQImage = cvMatToQImage(cropped);

        // Update both original and processed - this is a destructive operation
        m_currentDocument->setOriginalImage(croppedQImage);
        m_currentDocument->setProcessedImage(croppedQImage);

        // Update the pipeline with the new base image
        m_imagePipeline->setImg(cropped);

        m_currentDocument->setModified(true);
        Q_EMIT imageTransformed();
        qDebug() << "Image cropped to" << cropArea;
    } else {
        Q_EMIT errorOccurred("Failed to crop image");
    }
}
