#pragma once

#include <QObject>
#include <memory>

#include "AdjustmentSettings.h"
#include "ImageDocument.h"

// Forward declaration for ImagePipeline
class ImagePipeline;

/**
 * @brief Manages document lifecycle and coordinates model components
 *
 * Provides a facade for the Model layer, managing the current document
 * and its associated adjustment settings. Integrates with ImagePipeline
 * for real image processing.
 */
class DocumentManager : public QObject {
    Q_OBJECT

   public:
    explicit DocumentManager(QObject* parent = nullptr);
    ~DocumentManager();

    // Document access
    ImageDocument* currentDocument() const {
        return m_currentDocument.get();
    }
    AdjustmentSettings* adjustments() const {
        return m_adjustments.get();
    }

    // Document state
    bool hasDocument() const;
    bool hasUnsavedChanges() const;
    QString currentFilePath() const;
    QString currentFileName() const;

   public Q_SLOTS:
    // Document lifecycle
    bool openDocument(const QString& filePath);
    bool saveDocument();
    bool saveDocumentAs(const QString& filePath);
    void closeDocument();
    void newDocument(int width = 1920, int height = 1080);

    // Apply adjustments to generate processed image
    void applyAdjustments();

Q_SIGNALS:
    void documentOpened(const QString& filePath);
    void documentSaved(const QString& filePath);
    void documentClosed();
    void documentCreated();
    void documentStateChanged();
    void errorOccurred(const QString& error);

   private:
    std::unique_ptr<ImageDocument> m_currentDocument;
    std::unique_ptr<AdjustmentSettings> m_adjustments;
    std::unique_ptr<ImagePipeline> m_imagePipeline;
};
