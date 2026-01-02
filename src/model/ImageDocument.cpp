#include "ImageDocument.h"

#include <QFileInfo>

ImageDocument::ImageDocument(QObject* parent) : QObject(parent) {}

QString ImageDocument::fileName() const {
    if (m_filePath.isEmpty()) {
        return QString("Untitled");
    }
    return QFileInfo(m_filePath).fileName();
}

void ImageDocument::setFilePath(const QString& path) {
    if (m_filePath != path) {
        m_filePath = path;

        // Extract format from file extension
        if (!path.isEmpty()) {
            QFileInfo info(path);
            m_format = info.suffix().toUpper();
        }

        Q_EMIT filePathChanged(path);
    }
}

void ImageDocument::setOriginalImage(const QImage& image) {
    m_originalImage = image;
    m_processedImage = image;  // Initially, processed = original
    Q_EMIT originalImageChanged(image);
    Q_EMIT processedImageChanged(image);
}

void ImageDocument::setProcessedImage(const QImage& image) {
    if (m_processedImage != image) {
        m_processedImage = image;
        Q_EMIT processedImageChanged(image);
    }
}

void ImageDocument::setModified(bool modified) {
    if (m_isModified != modified) {
        m_isModified = modified;
        Q_EMIT modifiedChanged(modified);
    }
}

void ImageDocument::setFormat(const QString& format) {
    m_format = format;
}

void ImageDocument::clear() {
    m_filePath.clear();
    m_format.clear();
    m_originalImage = QImage();
    m_processedImage = QImage();
    m_isModified = false;
    Q_EMIT documentCleared();
}
