#include "DocumentManager.h"

#include <QDebug>
#include <QFileInfo>
#include <QImage>

#include "FourPointQuad.h"  // Four-point perspective crop data structure

// ImagePipeline and operations from image_processing component
// Note: include paths are relative to image_processing's PUBLIC include directories
// These headers include Halide.h internally, but QT_NO_KEYWORDS is defined
// which prevents Qt's emit/signals/slots from conflicting with Halide's emit
#include "color/saturation_adjust.h"
#include "color/tint_magenta.h"
#include "color/white_balance.h"
#include "denoise/denoise.h"  // Denoise operation
#include "effects/gray_image.h"
#include "effects/vintage1.h"
#include "geometry/crop.h"
#include "geometry/flip.h"
#include "geometry/perspective_crop.h"  // Four-point perspective crop
#include "geometry/rotate.h"
#include "image_controller.h"  // ImageController + ImageState
#include "image_resize.h"      // Resize operation (from utils/)
#include "light/auto_light.h"
#include "light/black_adjust.h"
#include "light/brightness_adjust.h"
#include "light/contrast_adjust.h"
#include "light/exposure_adjust.h"
#include "light/highlight_adjust.h"
#include "light/shadow_adjust.h"
#include "light/white_adjust.h"
#include "style_transfer/style_transfer.h"

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
      m_imageController(std::make_unique<ImageController>()),
      m_currentImageState(std::make_unique<ImageState>()),
      m_debounceTimer(new QTimer(this)),
      m_debouncedMode(false) {
    // Setup debounce timer
    m_debounceTimer->setSingleShot(true);
    connect(m_debounceTimer, &QTimer::timeout, this, &DocumentManager::applyAdjustments);

    // Connect adjustment changes to document modified state
    connect(m_adjustments.get(), &AdjustmentSettings::anySettingChanged, this, [this]() {
        if (m_currentDocument && !m_currentDocument->originalImage().isNull()) {
            m_currentDocument->setModified(true);
            if (m_debouncedMode) {
                applyAdjustmentsDebounced();
            } else {
                applyAdjustments();
            }
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
    *m_currentImageState = ImageState{};  // Reset image state
    *m_currentImageState = ImageState{};  // Reset image state
    m_undoStack.clear();
    m_redoStack.clear();
    Q_EMIT historyChanged(QStringList());

    // Set up new document
    m_currentDocument->setFilePath(filePath);
    m_currentDocument->setFormat(fileInfo.suffix().toLower());
    m_currentDocument->setOriginalImage(image);
    m_currentDocument->setProcessedImage(image);  // Start with unmodified
    m_currentDocument->setModified(false);

    // Set the image in the controller for processing
    cv::Mat cvImage = qImageToCvMat(image);
    m_imageController->setImage(cvImage);
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
    *m_currentImageState = ImageState{};  // Reset image state
    m_undoStack.clear();
    m_redoStack.clear();

    Q_EMIT documentClosed();
    Q_EMIT documentStateChanged();
    Q_EMIT historyChanged(QStringList());
    updateUndoRedoState();
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

    // Ensure image is loaded in controller
    if (m_imageController->getPipeline().getImg().empty()) {
        qDebug() << "No image in controller, reloading from document";
        cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());
        m_imageController->setImage(cvImage);
    }

    // Build ImageState from AdjustmentSettings
    // Map AdjustmentSettings (-100 to +100 int) to ImageState (float)
    m_currentImageState->brightness = static_cast<float>(m_adjustments->brightness());
    m_currentImageState->contrast = static_cast<float>(m_adjustments->contrast());
    m_currentImageState->exposure =
        static_cast<float>(m_adjustments->exposure()) / 20.0f;  // Map to -5.0 to +5.0
    m_currentImageState->highlight = static_cast<float>(m_adjustments->highlights());
    m_currentImageState->shadow = static_cast<float>(m_adjustments->shadows());
    m_currentImageState->white = static_cast<float>(m_adjustments->whites());
    m_currentImageState->black = static_cast<float>(m_adjustments->blacks());
    m_currentImageState->temp = static_cast<float>(m_adjustments->temperature());
    m_currentImageState->tint = static_cast<float>(m_adjustments->tint());
    m_currentImageState->saturation = static_cast<float>(m_adjustments->saturation());
    m_currentImageState->clarity = static_cast<float>(m_adjustments->clarity());
    m_currentImageState->sharpen = static_cast<float>(m_adjustments->sharpening());

    // Update controller with new state and process
    m_imageController->update(*m_currentImageState);
    cv::Mat result = m_imageController->process();

    // Apply current filter on top of adjustments if one is active
    if (!result.empty() && !m_currentFilter.isEmpty()) {
        try {
            if (m_currentFilter == "Grayscale") {
                GrayImage grayFilter;
                result = grayFilter.apply(result);
            } else if (m_currentFilter == "Vintage") {
                Vintage1 vintageFilter;
                result = vintageFilter.apply(result);
            }
        } catch (const std::exception& e) {
            qDebug() << "Filter application error:" << e.what();
        }
    }

    // Apply denoise if value > 0 (applied after filters for noise-free final output)
    if (!result.empty() && m_adjustments->denoise() > 0) {
        try {
            Denoise denoiseOp(m_adjustments->denoise());
            result = denoiseOp.apply(result);
        } catch (const std::exception& e) {
            qDebug() << "Denoise error:" << e.what();
        }
    }

    if (!result.empty()) {
        QImage processedQImage = cvMatToQImage(result);
        m_currentDocument->setProcessedImage(processedQImage);
        qDebug() << "Image processed via ImageController"
                 << (m_currentFilter.isEmpty() ? "" : "+ filter: " + m_currentFilter);
    } else {
        qDebug() << "ImageController processing returned empty result, keeping original";
        m_currentDocument->setProcessedImage(m_currentDocument->originalImage());
    }
}

void DocumentManager::applyAdjustmentsDebounced() {
    // Restart the timer - processing will happen when timer fires
    m_debounceTimer->start(DEBOUNCE_DELAY_MS);
}

void DocumentManager::setDebouncedMode(bool enabled) {
    m_debouncedMode = enabled;
    if (!enabled && m_debounceTimer->isActive()) {
        // If disabling debounce while timer is active, apply immediately
        m_debounceTimer->stop();
        applyAdjustments();
    }
}

bool DocumentManager::applyAdjustmentsPermanently() {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to apply corrections to");
        return false;
    }

    // Save state for undo before making permanent changes
    saveStateToHistory("Apply Adjustments");

    // Get the current processed image and make it the new original
    QImage processedImage = m_currentDocument->processedImage();
    if (processedImage.isNull()) {
        Q_EMIT errorOccurred("No processed image available");
        return false;
    }

    // Set the processed image as the new original
    m_currentDocument->setOriginalImage(processedImage);
    m_currentDocument->setProcessedImage(processedImage);

    // Reset all adjustments to zero since they're now baked in
    m_adjustments->resetAll();

    qDebug() << "Adjustments applied permanently - image is now the base";
    Q_EMIT documentStateChanged();
    return true;
}

AutoLightSettings DocumentManager::estimateAutoLight() {
    if (!hasDocument()) {
        return AutoLightSettings{};
    }

    // Get current original image
    QImage originalImage = m_currentDocument->originalImage();
    cv::Mat srcMat = qImageToCvMat(originalImage);

    try {
        // Analyze image for optimal settings
        return AutoLight::analyze(srcMat);
    } catch (const std::exception& e) {
        qDebug() << "AutoLight analysis error:" << e.what();
        return AutoLightSettings{};
    }
}

void DocumentManager::setGpuMode(bool enabled) {
    m_imageController->getPipeline().setFusionMode(enabled);
    qDebug() << "Processing mode set to:" << (enabled ? "GPU (Fusion)" : "CPU (Sequential)");

    // Reprocess current image with new mode if we have a document
    if (hasDocument()) {
        applyAdjustments();
    }
}

bool DocumentManager::isGpuMode() const {
    return m_imageController->getPipeline().isFusionMode();
}

void DocumentManager::rotateImage(int degrees) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to rotate");
        return;
    }

    // Save state for undo before making changes
    saveStateToHistory(QString("Rotate %1°").arg(degrees));

    // Get the current image (always use original for destructive ops to avoid baking in
    // adjustments)
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());

    // Apply rotation
    auto rotateOp = std::make_shared<Rotate>(degrees);
    cv::Mat rotated = rotateOp->apply(cvImage);

    if (!rotated.empty()) {
        QImage rotatedQImage = cvMatToQImage(rotated);

        // Update both original and processed - this is a destructive operation
        m_currentDocument->setOriginalImage(rotatedQImage);
        m_currentDocument->setProcessedImage(rotatedQImage);

        // Update the pipeline with the new base image
        m_imageController->setImage(rotated);

        m_currentDocument->setModified(true);
        Q_EMIT imageTransformed();
        qDebug() << "Image rotated by" << degrees << "degrees";
    } else {
        // Remove the saved state if operation failed
        if (!m_undoStack.isEmpty())
            m_undoStack.pop();
        Q_EMIT errorOccurred("Failed to rotate image");
    }
}

void DocumentManager::flipImage(int direction) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to flip");
        return;
    }

    // Save state for undo before making changes
    QString dirStr = (direction == 1) ? "Horizontal" : "Vertical";
    saveStateToHistory(QString("Flip %1").arg(dirStr));

    // Get the current image
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());

    // Apply flip (0 = vertical, 1 = horizontal)
    auto flipOp = std::make_shared<Flip>(direction);
    cv::Mat flipped = flipOp->apply(cvImage);

    if (!flipped.empty()) {
        QImage flippedQImage = cvMatToQImage(flipped);

        // Update both original and processed - this is a destructive operation
        m_currentDocument->setOriginalImage(flippedQImage);
        m_currentDocument->setProcessedImage(flippedQImage);

        // Update the pipeline with the new base image
        m_imageController->setImage(flipped);

        m_currentDocument->setModified(true);
        Q_EMIT imageTransformed();
        qDebug() << "Image flipped" << (direction == 1 ? "horizontally" : "vertically");
    } else {
        // Remove the saved state if operation failed
        if (!m_undoStack.isEmpty())
            m_undoStack.pop();
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

    // Save state for undo before making changes
    saveStateToHistory("Crop");

    // Get the current image
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());

    // Validate crop area against image dimensions
    cv::Rect cvCropRect(cropArea.x(), cropArea.y(), cropArea.width(), cropArea.height());
    if (cvCropRect.x < 0 || cvCropRect.y < 0 || cvCropRect.x + cvCropRect.width > cvImage.cols ||
        cvCropRect.y + cvCropRect.height > cvImage.rows) {
        // Remove the saved state if validation fails
        if (!m_undoStack.isEmpty())
            m_undoStack.pop();
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
        m_imageController->setImage(cropped);

        m_currentDocument->setModified(true);
        Q_EMIT imageTransformed();
        qDebug() << "Image cropped to" << cropArea;
    } else {
        // Remove the saved state if operation failed
        if (!m_undoStack.isEmpty())
            m_undoStack.pop();
        Q_EMIT errorOccurred("Failed to crop image");
    }
}

void DocumentManager::perspectiveCropImage(const FourPointQuad& quad) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to crop");
        return;
    }

    // Save state for undo before making changes
    saveStateToHistory("Perspective Crop");

    // Get the current image
    cv::Mat cvImage = qImageToCvMat(m_currentDocument->originalImage());

    // Convert FourPointQuad (Qt) to PerspectiveCrop::QuadPoints (OpenCV)
    PerspectiveCrop::QuadPoints quadPoints;
    quadPoints.topLeft =
        cv::Point2f(static_cast<float>(quad.topLeft.x()), static_cast<float>(quad.topLeft.y()));
    quadPoints.topRight =
        cv::Point2f(static_cast<float>(quad.topRight.x()), static_cast<float>(quad.topRight.y()));
    quadPoints.bottomRight = cv::Point2f(static_cast<float>(quad.bottomRight.x()),
                                         static_cast<float>(quad.bottomRight.y()));
    quadPoints.bottomLeft = cv::Point2f(static_cast<float>(quad.bottomLeft.x()),
                                        static_cast<float>(quad.bottomLeft.y()));

    // Apply perspective crop
    try {
        PerspectiveCrop perspectiveOp(quadPoints);
        cv::Mat result = perspectiveOp.apply(cvImage);

        if (!result.empty()) {
            QImage resultQImage = cvMatToQImage(result);

            // Update both original and processed - this is a destructive operation
            m_currentDocument->setOriginalImage(resultQImage);
            m_currentDocument->setProcessedImage(resultQImage);

            // Update the pipeline with the new base image
            m_imageController->setImage(result);

            m_currentDocument->setModified(true);
            Q_EMIT imageTransformed();
            qDebug() << "Perspective crop applied with quad:"
                     << "TL(" << quad.topLeft.x() << "," << quad.topLeft.y() << ")"
                     << "TR(" << quad.topRight.x() << "," << quad.topRight.y() << ")"
                     << "BR(" << quad.bottomRight.x() << "," << quad.bottomRight.y() << ")"
                     << "BL(" << quad.bottomLeft.x() << "," << quad.bottomLeft.y() << ")";
        } else {
            if (!m_undoStack.isEmpty())
                m_undoStack.pop();
            Q_EMIT errorOccurred("Failed to apply perspective crop");
        }
    } catch (const std::exception& e) {
        if (!m_undoStack.isEmpty())
            m_undoStack.pop();
        Q_EMIT errorOccurred(QString("Perspective crop error: %1").arg(e.what()));
    }
}

void DocumentManager::resizeImage(int width, int height) {
    if (!hasDocument()) {
        Q_EMIT errorOccurred("No document to resize");
        return;
    }

    // Save state for undo
    saveStateToHistory(QString("Resize %1x%2").arg(width).arg(height));

    // Get current image
    QImage currentImage = m_currentDocument->originalImage();
    cv::Mat srcMat = qImageToCvMat(currentImage);

    try {
        // Use ResizeImage operation from image_processing
        ResizeImage resizeOp(static_cast<uint>(height), static_cast<uint>(width));
        cv::Mat resized = resizeOp.apply(srcMat);

        if (!resized.empty()) {
            QImage resizedImage = cvMatToQImage(resized);
            m_currentDocument->setOriginalImage(resizedImage);
            m_currentDocument->setProcessedImage(resizedImage);

            // Update pipeline with resized image
            m_imageController->setImage(resized);

            m_currentDocument->setModified(true);
            Q_EMIT imageTransformed();
            qDebug() << "Image resized to" << width << "x" << height;
        } else {
            if (!m_undoStack.isEmpty())
                m_undoStack.pop();
            Q_EMIT errorOccurred("Resize operation failed");
        }
    } catch (const std::exception& e) {
        if (!m_undoStack.isEmpty())
            m_undoStack.pop();
        Q_EMIT errorOccurred(QString("Resize error: %1").arg(e.what()));
    }
}

void DocumentManager::applyFilter(const QString& filterName) {
    if (!hasDocument()) {
        qDebug() << "Cannot apply filter - no document";
        return;
    }

    qDebug() << "Applying filter:" << filterName;

    // Save state for undo
    saveStateToHistory(QString("Filter: %1").arg(filterName));

    // Get current image
    QImage originalImage = m_currentDocument->originalImage();
    cv::Mat srcMat = qImageToCvMat(originalImage);
    cv::Mat resultMat;

    try {
        if (filterName == "Grayscale") {
            GrayImage grayFilter;
            resultMat = grayFilter.apply(srcMat);
        } else if (filterName == "Vintage") {
            Vintage1 vintageFilter;
            resultMat = vintageFilter.apply(srcMat);
        } else if (filterName == "AutoEnhance") {
            // Apply auto light enhancement
            AutoLightSettings autoSettings = AutoLight::analyze(srcMat);

            // Apply each adjustment operation sequentially
            resultMat = srcMat.clone();

            // Apply exposure if significant
            if (std::abs(autoSettings.exposure) > 0.01f) {
                AdjustExposure exposure(autoSettings.exposure);
                resultMat = exposure.apply(resultMat);
            }

            // Apply contrast if significant
            if (std::abs(autoSettings.contrast) > 1.0f) {
                AdjustContrast contrast(static_cast<int>(autoSettings.contrast));
                resultMat = contrast.apply(resultMat);
            }
        } else if (filterName.startsWith("StyleTransfer_")) {
            // AI Style Transfer
            StyleType styleType = StyleType::Mosaic;  // Default

            if (filterName.contains("Candy")) {
                styleType = StyleType::Candy;
            } else if (filterName.contains("RainPrincess")) {
                styleType = StyleType::RainPrincess;
            } else if (filterName.contains("Udnie")) {
                styleType = StyleType::Udnie;
            } else if (filterName.contains("Pointillism")) {
                styleType = StyleType::Pointillism;
            }

            StyleTransfer styleTransfer(styleType);
            styleTransfer.setStrength(m_styleStrength);
            resultMat = styleTransfer.apply(srcMat);
        } else {
            qDebug() << "Unknown filter:" << filterName;
            return;
        }

        if (!resultMat.empty()) {
            QImage resultImage = cvMatToQImage(resultMat);
            m_currentDocument->setProcessedImage(resultImage);
            m_currentDocument->setModified(true);

            // Track persistent filters (Grayscale, Vintage) so they persist during adjustments
            if (filterName == "Grayscale" || filterName == "Vintage") {
                m_currentFilter = filterName;
            }

            // Notify UI about filter change (e.g., to disable color controls for Grayscale)
            Q_EMIT filterChanged(filterName);

            Q_EMIT imageTransformed();
            qDebug() << "Filter applied successfully:" << filterName;
        } else {
            qDebug() << "Filter returned empty result:" << filterName;
            // Restore from undo stack
            if (!m_undoStack.isEmpty()) {
                m_undoStack.pop();
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "Filter error:" << e.what();
        Q_EMIT errorOccurred(QString("Filter error: %1").arg(e.what()));
        // Restore from undo stack
        if (!m_undoStack.isEmpty()) {
            m_undoStack.pop();
        }
    }
}

void DocumentManager::removeFilter() {
    if (!hasDocument()) {
        return;
    }

    // Clear the current filter tracking
    m_currentFilter.clear();

    // Notify UI that filter was removed (re-enable controls)
    Q_EMIT filterChanged("");

    // Reset to original image
    QImage originalImage = m_currentDocument->originalImage();
    m_currentDocument->setProcessedImage(originalImage);

    // Reset pipeline
    cv::Mat cvImage = qImageToCvMat(originalImage);
    m_imageController->setImage(cvImage);

    // Reset all adjustments
    if (m_adjustments) {
        m_adjustments->resetAll();
    }

    Q_EMIT imageTransformed();
    qDebug() << "Filter removed, restored original image";
}

bool DocumentManager::canUndo() const {
    return !m_undoStack.isEmpty();
}

bool DocumentManager::canRedo() const {
    return !m_redoStack.isEmpty();
}

QStringList DocumentManager::getHistory() const {
    QStringList list;
    for (int i = m_undoStack.size() - 1; i >= 0; --i) {
        list.append(m_undoStack[i].description);
    }
    return list;
}

void DocumentManager::saveAdjustmentState(const QString& name, int value) {
    if (!hasDocument())
        return;

    // For slider adjustments, we save the state locally to avoid spamming the stack.
    // However, since we don't have a "Previous State" mechanism yet, we will
    // rely on the fact that simple adjustment undo/redo is acceptable to restore
    // the snapshot of settings *at that point*.

    // NOTE: Ideally we should save the state BEFORE the drag started.
    // Capturing it at release time means we capture the NEW value.
    // This is a known limitation to be addressed in the next iteration if needed.
    // For now, consistent snapshots allow jumping between states.

    saveStateToHistory(QString("%1: %2").arg(name).arg(value));
}

void DocumentManager::saveStateToHistory(const QString& description) {
    if (!hasDocument())
        return;

    HistoryState state;
    state.image = m_currentDocument->originalImage().copy();
    state.adjustments = m_adjustments->createSnapshot();
    state.description = description;

    // Save current state to undo stack
    m_undoStack.push(state);

    // Clear redo stack when new action is performed
    m_redoStack.clear();

    // Limit history size
    while (m_undoStack.size() > MAX_HISTORY_SIZE) {
        m_undoStack.remove(0);
    }

    updateUndoRedoState();
    Q_EMIT historyChanged(getHistory());
}

void DocumentManager::updateUndoRedoState() {
    Q_EMIT undoRedoStateChanged(canUndo(), canRedo());
}

void DocumentManager::undo() {
    if (!canUndo() || !hasDocument()) {
        qDebug() << "Cannot undo - no history";
        return;
    }

    // Capture current state for Redo
    HistoryState currentState;
    currentState.image = m_currentDocument->originalImage().copy();
    currentState.adjustments = m_adjustments->createSnapshot();
    if (!m_undoStack.isEmpty()) {
        currentState.description = m_undoStack.top().description;
    }
    m_redoStack.push(currentState);

    // Restore previous state
    HistoryState previousState = m_undoStack.pop();

    // Restore base image
    m_currentDocument->setOriginalImage(previousState.image);

    // Restore adjustments
    m_adjustments->applySnapshot(previousState.adjustments);

    // Re-process image with restored settings
    applyAdjustments();

    m_currentDocument->setModified(true);
    Q_EMIT imageTransformed();
    updateUndoRedoState();
    Q_EMIT historyChanged(getHistory());
    qDebug() << "Undo performed, remaining history:" << m_undoStack.size();
}

void DocumentManager::redo() {
    if (!canRedo() || !hasDocument()) {
        qDebug() << "Cannot redo - no history";
        return;
    }

    // Save current state to Undo
    HistoryState currentState;
    currentState.image = m_currentDocument->originalImage().copy();
    currentState.adjustments = m_adjustments->createSnapshot();
    if (!m_redoStack.isEmpty()) {
        currentState.description = m_redoStack.top().description;
    }
    m_undoStack.push(currentState);

    // Restore next state
    HistoryState nextState = m_redoStack.pop();

    m_currentDocument->setOriginalImage(nextState.image);
    m_adjustments->applySnapshot(nextState.adjustments);

    applyAdjustments();

    m_currentDocument->setModified(true);
    Q_EMIT imageTransformed();
    updateUndoRedoState();
    Q_EMIT historyChanged(getHistory());
}

void DocumentManager::setStyleStrength(float strength) {
    if (std::abs(m_styleStrength - strength) < 0.01f)
        return;

    m_styleStrength = strength;

    // If we are currently using a style transfer filter, reapply it
    if (m_currentFilter.startsWith("StyleTransfer_")) {
        applyFilter(m_currentFilter);
    }
}
