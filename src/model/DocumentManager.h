#pragma once

#include <QObject>
#include <QPointF>
#include <QRect>
#include <QStack>
#include <QTimer>
#include <memory>

#include "../image_processing/operations/light/auto_light.h"  // For AutoLightSettings struct
#include "AdjustmentSettings.h"
#include "FourPointQuad.h"  // Four-point perspective crop
#include "ImageDocument.h"
class ImageController;
struct ImageState;

struct HistoryState {
    QImage image;
    AdjustmentSettings::Snapshot adjustments;
    QString description;
    QString filter;  // Active filter name (for undo/redo)

    // Helper to check if only adjustments changed (for memory optimization if needed later)
    bool sameImage(const HistoryState& other) const {
        return image.cacheKey() == other.image.cacheKey();
    }
};

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
    // Reloads original image from file and resets all adjustments
    void resetToOriginal();

    // Apply adjustments to generate processed image
    void applyAdjustments();
    void applyAdjustmentsDebounced();     // Debounced version for slider dragging
    void setDebouncedMode(bool enabled);  // Enable/disable debouncing
    bool applyAdjustmentsPermanently();   // Bake adjustments into base image

    // Auto-Light estimation (returns settings without applying them)
    AutoLightSettings estimateAutoLight();              // Analyze current processed image
    AutoLightSettings estimateAutoLightFromOriginal();  // Analyze original image

    // Processing mode (CPU vs GPU)
    void setGpuMode(bool enabled);  // Toggle between CPU and GPU processing
    bool isGpuMode() const;         // Returns true if using GPU (fusion) mode

    // Geometry operations (destructive - modify the base image)
    void rotateImage(int degrees);
    void applyStraighten(float angle, const QRect& cropRect);  // Destructive straighten
    void flipImage(int direction);                             // 0 = vertical, 1 = horizontal
    void cropImage(const QRect& cropArea);
    void perspectiveCropImage(const FourPointQuad& quad);  // Four-point perspective crop
    void resizeImage(int width, int height);               // Resize image to new dimensions

    // Filter operations
    void applyFilter(const QString& filterName);
    void removeFilter();
    // Style transfer variation parameters
    void setStyleHueVariation(int value);
    void setStyleSatVariation(int value);
    void setStyleContrastVariation(int value);
    void setStyleNoiseAmount(int value);

    // Save current adjustment state to history (for slider release)
    // Save current adjustment state to history (for slider release)
    void beginInteraction();  // Capture state before starting an interaction
    void saveAdjustmentState(const QString& name, int value);

    // Undo/Redo operations
    void undo();
    void redo();

    // Get current history list (most recent first)
    QStringList getHistory() const;

   Q_SIGNALS:
    void documentOpened(const QString& filePath);
    void documentSaved(const QString& filePath);
    void documentClosed();
    void documentCreated();
    void documentStateChanged();
    void imageTransformed();                     // Emitted after rotate/flip/crop
    void errorOccurred(const QString& message);  // Error reporting
    void undoRedoStateChanged(bool canUndo, bool canRedo);
    void historyChanged(const QStringList& history);  // Emitted when history stack changes
    void filterChanged(const QString& filterName);    // Emitted when filter is applied/removed
    // Emitted after undo/redo so UI sliders can sync
    void adjustmentsRestored(int brightness, int contrast, int saturation, int exposure,
                             int highlights, int shadows, int whites, int blacks, int temperature,
                             int tint, int denoise, int clarity, int sharpening);
    void rotationRestored(int rotation);  // Separate signal for rotation slider

   private:
    void saveStateToHistory(const QString& description = "Adjustment");
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
    QStack<HistoryState> m_undoStack;
    QStack<HistoryState> m_redoStack;
    static const int MAX_HISTORY_SIZE = 20;

    // Active filter tracking (persists across adjustment changes)
    QString m_currentFilter;  // Empty means no filter applied

    // Style transfer variation parameters
    int m_styleHueVariation = 0;
    int m_styleSatVariation = 0;
    int m_styleContrastVariation = 0;
    int m_styleNoiseAmount = 0;

    // Interaction state tracking
    HistoryState m_interactionStartState;
    bool m_hasInteractionState = false;
};
