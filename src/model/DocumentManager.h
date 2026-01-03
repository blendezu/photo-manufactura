#pragma once

#include <QObject>
#include <QRect>
#include <QStack>
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

    // Undo/Redo state
    bool canUndo() const;
    bool canRedo() const;

   public Q_SLOTS:
    // Document lifecycle
    bool openDocument(const QString& filePath);
    bool saveDocument();
    bool saveDocumentAs(const QString& filePath);
    void closeDocument();
    void newDocument(int width = 1920, int height = 1080);

    // Apply adjustments to generate processed image
    void applyAdjustments();

    // Geometry operations (destructive - modify the base image)
    void rotateImage(int degrees);
    void flipImage(int direction);  // 0 = vertical, 1 = horizontal
    void cropImage(const QRect& cropArea);

    // Filter operations
    void applyFilter(const QString& filterName);
    void removeFilter();

    // Undo/Redo operations
    void undo();
    void redo();

   Q_SIGNALS:
    void documentOpened(const QString& filePath);
    void documentSaved(const QString& filePath);
    void documentClosed();
    void documentCreated();
    void documentStateChanged();
    void imageTransformed();                     // Emitted after rotate/flip/crop
    void errorOccurred(const QString& message);  // Error reporting
    void undoRedoStateChanged(bool canUndo, bool canRedo);

   private:
    void saveStateToHistory();
    void updateUndoRedoState();

    std::unique_ptr<ImageDocument> m_currentDocument;
    std::unique_ptr<AdjustmentSettings> m_adjustments;
    std::unique_ptr<ImagePipeline> m_imagePipeline;

    // Undo/Redo history
    QStack<QImage> m_undoStack;
    QStack<QImage> m_redoStack;
    static const int MAX_HISTORY_SIZE = 20;
};
