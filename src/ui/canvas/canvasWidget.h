#pragma once

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QWheelEvent>

class CanvasWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    ~CanvasWidget() override;

    void setImage(const QImage& image);
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    double getZoomFactor() const {
        return m_zoomFactor;
    }

   signals:
    void imageClicked(QPoint position);
    void zoomChanged(double factor);

   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

   private:
    // OpenGL Resources
    QOpenGLTexture* texture;
    QOpenGLShaderProgram* m_shaderProgram;
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

    // Image properties
    QSize m_imageSize;

    // Constants
    static constexpr double MIN_ZOOM = 0.1;
    static constexpr double MAX_ZOOM = 10.0;
    static constexpr double ZOOM_STEP = 1.2;

    // Helper methods
    void setupShaders();
    void setupGeometry();
    void updateMatrices();
};
