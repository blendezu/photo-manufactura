#pragma once

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QRect>
#include <QWheelEvent>
#include <memory>

// Zoom mode types
enum class ZoomMode {
    None,  // Normal panning mode
    Zoom   // Click to zoom in, Alt+Click to zoom out
};

// Crop mode types
enum class CropType {
    Free,        // Manual free-form crop
    FixedSize,   // Fixed pixel dimensions
    AspectRatio  // Fixed aspect ratio
};

// Common aspect ratio presets
enum class AspectRatioPreset {
    Free,
    Square_1_1,
    Photo_4_3,
    Photo_3_2,
    Widescreen_16_9,
    Widescreen_21_9,
    Portrait_3_4,
    Portrait_2_3,
    Portrait_9_16,
    Custom
};

class CanvasWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    ~CanvasWidget() override;

    void setImage(const QImage& image);
    void resetView();  // Reset zoom and pan to center the image
    double getZoomFactor() const {
        return m_zoomFactor;
    }

    // Zoom mode
    void setZoomMode(ZoomMode mode);
    ZoomMode getZoomMode() const {
        return m_zoomMode;
    }

    // Crop mode
    void setCropMode(bool enabled);
    bool isCropMode() const {
        return m_cropMode;
    }
    QRect getCropSelection() const;  // Returns selection in image coordinates

    // Crop options
    void setCropType(CropType type);
    CropType getCropType() const {
        return m_cropType;
    }

    void setFixedCropSize(const QSize& size);
    QSize getFixedCropSize() const {
        return m_fixedCropSize;
    }

    void setAspectRatioPreset(AspectRatioPreset preset);
    AspectRatioPreset getAspectRatioPreset() const {
        return m_aspectPreset;
    }

    void setCustomAspectRatio(double widthRatio, double heightRatio);
    double getAspectRatio() const {
        return m_aspectRatio;
    }

    // Before/After comparison
    void setOriginalImage(const QImage& image);
    void setCompareMode(bool enabled);
    bool isCompareMode() const {
        return m_compareMode;
    }
    void setCompareSplitPosition(double position);  // 0.0 to 1.0

   public Q_SLOTS:
    // Receive zoom level from controller (0.1 to 10.0)
    void setZoomLevel(double level);
    void applyCrop();          // Emit crop signal with current selection
    void cancelCrop();         // Exit crop mode without applying
    void toggleCompareMode();  // Toggle before/after comparison

   Q_SIGNALS:
    void imageClicked(QPoint position);
    // Request zoom changes from controller
    void zoomInRequested();
    void zoomOutRequested();
    void fitToWindowRequested();
    void zoomModeChanged(ZoomMode mode);
    void zoomLevelChanged(double level);  // Current zoom level
    // Crop signals
    void cropRequested(const QRect& cropArea);
    void cropModeChanged(bool enabled);
    void cropTypeChanged(CropType type);
    // Compare mode
    void compareModeChanged(bool enabled);

   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    // OpenGL Resources (smart pointers for automatic cleanup)
    std::unique_ptr<QOpenGLTexture> m_texture;
    std::unique_ptr<QOpenGLShaderProgram> m_shaderProgram;
    QOpenGLVertexArrayObject m_quadVAO;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_indexBuffer;

    // Transformation matrices
    QMatrix4x4 m_mvpMatrix;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;

    // View state
    double m_zoomFactor;
    QPointF m_panOffset;
    QPoint m_lastPanPoint;
    bool m_panning;
    ZoomMode m_zoomMode = ZoomMode::None;

    // Image properties
    QSize m_imageSize;

    // Before/After comparison
    std::unique_ptr<QOpenGLTexture> m_originalTexture;
    bool m_compareMode = false;
    double m_compareSplitPosition = 0.5;  // 0.0 = all original, 1.0 = all processed
    bool m_draggingSplit = false;

    // Crop mode state
    bool m_cropMode = false;
    QRect m_cropSelection;    // Selection in IMAGE coordinates (ready for crop)
    QPoint m_cropStartPoint;  // In widget coordinates for interaction
    bool m_selecting = false;
    bool m_showRuleOfThirds = true;

    // Crop options
    CropType m_cropType = CropType::Free;
    QSize m_fixedCropSize = QSize(800, 600);  // Default fixed size
    AspectRatioPreset m_aspectPreset = AspectRatioPreset::Free;
    double m_aspectRatio = 0.0;  // 0 = free, otherwise width/height ratio

    // Constants
    static constexpr double MIN_ZOOM = 0.1;
    static constexpr double MAX_ZOOM = 10.0;
    static constexpr double ZOOM_STEP = 1.2;

    // Helper methods
    void setupShaders();
    void setupGeometry();
    void updateMatrices();
    QPoint widgetToImageCoords(const QPoint& widgetPos) const;
    QPoint imageToWidgetCoords(const QPoint& imagePos) const;
    QRectF getDisplayedImageBounds() const;
    void drawCropOverlay(QPainter& painter);
    void drawCompareOverlay(QPainter& painter);

    // Crop helper methods
    QRect constrainCropSelection(const QPoint& startWidget, const QPoint& endWidget) const;
    double getPresetAspectRatio(AspectRatioPreset preset) const;
    QString getCropModeLabel() const;
};
