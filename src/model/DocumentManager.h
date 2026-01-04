#pragma once

#include <QObject>
#include <QPointF>
#include <QRect>
#include <QStack>
#include <QTimer>
#include <memory>

#include "AdjustmentSettings.h"
#include "FourPointQuad.h"  // Four-point perspective crop
#include "ImageDocument.h"

// Forward declaration - actual include in cpp to avoid header dependency issues
class ImageController;
struct ImageState;

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
    void applyAdjustmentsDebounced();     // Debounced version for slider dragging
    void setDebouncedMode(bool enabled);  // Enable/disable debouncing
    bool applyAdjustmentsPermanently();   // Bake adjustments into base image

    // Processing mode (CPU vs GPU)
    void setGpuMode(bool enabled);  // Toggle between CPU and GPU processing
    bool isGpuMode() const;         // Returns true if using GPU (fusion) mode

    // Geometry operations (destructive - modify the base image)
    void rotateImage(int degrees);
    void flipImage(int direction);  // 0 = vertical, 1 = horizontal
    void cropImage(const QRect& cropArea);
    void perspectiveCropImage(const FourPointQuad& quad);  // Four-point perspective crop
    void resizeImage(int width, int height);               // Resize image to new dimensions

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
    void filterChanged(const QString& filterName);  // Emitted when filter is applied/removed

   private:
    void saveStateToHistory();
    void updateUndoRedoState();

    std::unique_ptr<ImageDocument> m_currentDocument;
    std::unique_ptr<AdjustmentSettings> m_adjustments;
    std::unique_ptr<ImageController> m_imageController;
    std::unique_ptr<ImageState> m_currentImageState;  // Tracks current processing state

    // Debouncing
    QTimer* m_debounceTimer;
    bool m_debouncedMode = false;
    static const int DEBOUNCE_DELAY_MS = 100;

    // Undo/Redo history
    QStack<QImage> m_undoStack;
    QStack<QImage> m_redoStack;
    static const int MAX_HISTORY_SIZE = 20;

    // Active filter tracking (persists across adjustment changes)
    QString m_currentFilter;  // Empty means no filter applied
};
