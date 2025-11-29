#include "DocumentManager.h"

#include <QFileInfo>
#include <QImage>

DocumentManager::DocumentManager(QObject* parent)
    : QObject(parent),
      m_currentDocument(std::make_unique<ImageDocument>(this)),
      m_adjustments(std::make_unique<AdjustmentSettings>(this)) {
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
        emit errorOccurred("No file path provided");
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        emit errorOccurred(QString("File does not exist: %1").arg(filePath));
        return false;
    }

    QImage image(filePath);
    if (image.isNull()) {
        emit errorOccurred(QString("Failed to load image: %1").arg(filePath));
        return false;
    }

    // Clear any previous state
    m_adjustments->resetAll();
    m_currentDocument->clear();

    // Set up new document
    m_currentDocument->setFilePath(filePath);
    m_currentDocument->setFormat(fileInfo.suffix().toLower());
    m_currentDocument->setOriginalImage(image);
    m_currentDocument->setProcessedImage(image);  // Start with unmodified
    m_currentDocument->setModified(false);

    emit documentOpened(filePath);
    emit documentStateChanged();
    return true;
}

bool DocumentManager::saveDocument() {
    if (!hasDocument()) {
        emit errorOccurred("No document to save");
        return false;
    }

    QString filePath = m_currentDocument->filePath();
    if (filePath.isEmpty()) {
        emit errorOccurred("No file path set - use Save As");
        return false;
    }

    return saveDocumentAs(filePath);
}

bool DocumentManager::saveDocumentAs(const QString& filePath) {
    if (!hasDocument()) {
        emit errorOccurred("No document to save");
        return false;
    }

    if (filePath.isEmpty()) {
        emit errorOccurred("No file path provided");
        return false;
    }

    const QImage& imageToSave = m_currentDocument->processedImage();
    if (imageToSave.isNull()) {
        emit errorOccurred("No image data to save");
        return false;
    }

    if (!imageToSave.save(filePath)) {
        emit errorOccurred(QString("Failed to save image: %1").arg(filePath));
        return false;
    }

    // Update document state
    m_currentDocument->setFilePath(filePath);
    QFileInfo fileInfo(filePath);
    m_currentDocument->setFormat(fileInfo.suffix().toLower());
    m_currentDocument->setModified(false);

    emit documentSaved(filePath);
    emit documentStateChanged();
    return true;
}

void DocumentManager::closeDocument() {
    m_adjustments->resetAll();
    m_currentDocument->clear();

    emit documentClosed();
    emit documentStateChanged();
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

    emit documentCreated();
    emit documentStateChanged();
}

void DocumentManager::applyAdjustments() {
    if (!hasDocument()) {
        return;
    }

    // TODO: Integrate with image_processing component
    // For now, just copy original to processed
    // In full implementation, this would:
    // 1. Take original image
    // 2. Apply all adjustments via ImagePipeline
    // 3. Set result as processed image

    QImage original = m_currentDocument->originalImage();
    QImage processed = original;  // Placeholder - actual processing goes here

    // Apply simple adjustments as a placeholder
    // Real implementation would use the image_processing component
    if (m_adjustments->hasAdjustments()) {
        // Brightness adjustment example (simplified)
        int brightness = m_adjustments->brightness();
        if (brightness != 0) {
            for (int y = 0; y < processed.height(); ++y) {
                for (int x = 0; x < processed.width(); ++x) {
                    QColor color = processed.pixelColor(x, y);
                    int r = qBound(0, color.red() + brightness, 255);
                    int g = qBound(0, color.green() + brightness, 255);
                    int b = qBound(0, color.blue() + brightness, 255);
                    processed.setPixelColor(x, y, QColor(r, g, b));
                }
            }
        }
    }

    m_currentDocument->setProcessedImage(processed);
}
