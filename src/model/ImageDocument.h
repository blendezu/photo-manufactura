#pragma once

#include <QImage>
#include <QObject>
#include <QString>

/**
 * @brief Represents an image document with its state
 *
 * This is the Model in the MVC pattern. It holds the document data
 * and notifies observers when data changes.
 */
class ImageDocument : public QObject {
    Q_OBJECT

   public:
    explicit ImageDocument(QObject* parent = nullptr);
    ~ImageDocument() = default;

    // File properties
    QString filePath() const {
        return m_filePath;
    }
    QString fileName() const;
    bool isModified() const {
        return m_isModified;
    }
    bool hasImage() const {
        return !m_originalImage.isNull();
    }

    // Image data
    QImage originalImage() const {
        return m_originalImage;
    }
    QImage processedImage() const {
        return m_processedImage;
    }
    QSize imageSize() const {
        return m_originalImage.size();
    }

    // Image metadata
    QString format() const {
        return m_format;
    }
    int width() const {
        return m_originalImage.width();
    }
    int height() const {
        return m_originalImage.height();
    }

   public Q_SLOTS:
    // Document operations
    void setFilePath(const QString& path);
    void setOriginalImage(const QImage& image);
    void setProcessedImage(const QImage& image);
    void setModified(bool modified);
    void setFormat(const QString& format);

    // Reset document
    void clear();

Q_SIGNALS:
    void filePathChanged(const QString& path);
    void originalImageChanged(const QImage& image);
    void processedImageChanged(const QImage& image);
    void modifiedChanged(bool modified);
    void documentCleared();

   private:
    QString m_filePath;
    QString m_format;
    QImage m_originalImage;
    QImage m_processedImage;
    bool m_isModified = false;
};
