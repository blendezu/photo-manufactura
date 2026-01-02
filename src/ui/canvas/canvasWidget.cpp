#include "canvasWidget.h"

#include <QDebug>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QPainter>

CanvasWidget::CanvasWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      m_texture(nullptr),
      m_shaderProgram(nullptr),
      m_zoomFactor(1.0),
      m_panOffset(0, 0),
      m_panning(false) {
    setFocusPolicy(Qt::StrongFocus);  // Enable keyboard focus
}

CanvasWidget::~CanvasWidget() {
    makeCurrent();
    // Reset smart pointers while OpenGL context is current
    m_texture.reset();
    m_shaderProgram.reset();
    doneCurrent();
}

void CanvasWidget::setImage(const QImage& image) {
    if (image.isNull())
        return;

    makeCurrent();

    // Clean up old texture
    m_texture.reset();

    // Create new texture
    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);
    m_texture = std::make_unique<QOpenGLTexture>(rgbaImage);
    m_texture->setMinificationFilter(QOpenGLTexture::Linear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);

    m_imageSize = image.size();

    // Reset view to center the image when dimensions change
    resetView();

    update();

    doneCurrent();
}

void CanvasWidget::resetView() {
    m_zoomFactor = 1.0;
    m_panOffset = QPointF(0, 0);
    updateMatrices();
}

void CanvasWidget::initializeGL() {
    initializeOpenGLFunctions();

    // Set clear color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Setup shaders and geometry
    setupShaders();
    setupGeometry();

    // Initialize matrices
    updateMatrices();
}

void CanvasWidget::setupShaders() {
    m_shaderProgram = std::make_unique<QOpenGLShaderProgram>(this);

    // Vertex shader (GLSL 120 for macOS compatibility)
    const char* vertexShaderSource = R"(
        #version 120
        attribute vec3 aPos;
        attribute vec2 aTexCoord;
        
        varying vec2 TexCoord;
        
        uniform mat4 u_mvpMatrix;
        
        void main() {
            gl_Position = u_mvpMatrix * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    // Fragment shader (GLSL 120 for macOS compatibility)
    const char* fragmentShaderSource = R"(
        #version 120
        varying vec2 TexCoord;
        uniform sampler2D u_texture;
        
        void main() {
            gl_FragColor = texture2D(u_texture, TexCoord);
        }
    )";

    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    // Bind attribute locations before linking
    m_shaderProgram->bindAttributeLocation("aPos", 0);
    m_shaderProgram->bindAttributeLocation("aTexCoord", 1);

    m_shaderProgram->link();

    if (!m_shaderProgram->isLinked()) {
        qDebug() << "Shader program failed to link:" << m_shaderProgram->log();
    }
}

void CanvasWidget::setupGeometry() {
    // Quad vertices (position + texture coordinates)
    float vertices[] = {
        // positions        // texture coords
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // bottom left
        1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,  // bottom right
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  // top right
        -1.0f, 1.0f,  0.0f, 0.0f, 0.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    // Setup VAO
    m_quadVAO.create();
    m_quadVAO.bind();

    // Setup vertex buffer
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(vertices, sizeof(vertices));

    // Setup index buffer
    m_indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(indices, sizeof(indices));

    // Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    m_quadVAO.release();
}

void CanvasWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    updateMatrices();
}

void CanvasWidget::updateMatrices() {
    // Projection matrix (orthographic)
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.ortho(-1, 1, -1, 1, -1, 1);

    // Model matrix (aspect ratio correction)
    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();

    if (!m_imageSize.isEmpty() && width() > 0 && height() > 0) {
        // Calculate aspect ratios
        double widgetAspect = (double)width() / height();
        double imageAspect = (double)m_imageSize.width() / m_imageSize.height();

        // Scale the quad to match image aspect ratio
        if (imageAspect > widgetAspect) {
            // Image is wider than widget - scale height
            modelMatrix.scale(1.0, widgetAspect / imageAspect, 1.0);
        } else {
            // Image is taller than widget - scale width
            modelMatrix.scale(imageAspect / widgetAspect, 1.0, 1.0);
        }
    }

    // View matrix (zoom and pan)
    m_viewMatrix.setToIdentity();
    m_viewMatrix.scale(m_zoomFactor);
    m_viewMatrix.translate(m_panOffset.x(), m_panOffset.y());

    // Combined MVP matrix
    m_mvpMatrix = m_projectionMatrix * m_viewMatrix * modelMatrix;
}

void CanvasWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_texture || !m_shaderProgram) {
        return;
    }

    // Use shader program
    m_shaderProgram->bind();

    // Bind texture to texture unit 0
    m_texture->bind(0);
    m_shaderProgram->setUniformValue("u_texture", 0);

    // Set transformation matrices for zoom/pan
    m_shaderProgram->setUniformValue("u_mvpMatrix", m_mvpMatrix);

    // Draw the quad
    m_quadVAO.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_quadVAO.release();

    // Cleanup
    m_texture->release();
    m_shaderProgram->release();

    // Draw crop overlay using QPainter (after OpenGL)
    if (m_cropMode) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        drawCropOverlay(painter);
        painter.end();
    }
}

void CanvasWidget::setZoomLevel(double level) {
    // Receive zoom level from controller and update rendering
    if (m_zoomFactor != level) {
        m_zoomFactor = level;
        updateMatrices();
        update();
    }
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    // Request zoom change from controller instead of changing directly
    if (event->angleDelta().y() > 0) {
        Q_EMIT zoomInRequested();
    } else {
        Q_EMIT zoomOutRequested();
    }
    event->accept();
}

void CanvasWidget::keyPressEvent(QKeyEvent* event) {
    if (m_cropMode) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            applyCrop();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Escape) {
            cancelCrop();
            event->accept();
            return;
        }
    }
    QOpenGLWidget::keyPressEvent(event);
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_cropMode) {
            // Start crop selection
            m_selecting = true;
            m_cropStartPoint = event->pos();
            m_cropSelection = QRect(m_cropStartPoint, QSize(0, 0));
            update();
        } else {
            // Normal panning
            m_panning = true;
            m_lastPanPoint = event->pos();
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_cropMode && m_selecting) {
        // Update crop selection rectangle
        m_cropSelection = QRect(m_cropStartPoint, event->pos()).normalized();
        update();
    } else if (m_panning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_panOffset +=
            QPointF(delta.x() / (width() * m_zoomFactor), -delta.y() / (height() * m_zoomFactor));
        m_lastPanPoint = event->pos();
        updateMatrices();
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_cropMode && m_selecting) {
            m_selecting = false;
            // Keep selection visible
            update();
        } else {
            m_panning = false;
        }
    }
}

// Crop mode methods
void CanvasWidget::setCropMode(bool enabled) {
    if (m_cropMode != enabled) {
        m_cropMode = enabled;
        m_selecting = false;
        m_cropSelection = QRect();
        setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
        if (enabled) {
            setFocus();  // Grab keyboard focus for Enter/Escape
        }
        Q_EMIT cropModeChanged(enabled);
        update();
    }
}

QRect CanvasWidget::getCropSelection() const {
    if (m_cropSelection.isEmpty() || m_imageSize.isEmpty()) {
        return QRect();
    }

    // Convert widget coordinates to image coordinates
    QPoint topLeft = widgetToImageCoords(m_cropSelection.topLeft());
    QPoint bottomRight = widgetToImageCoords(m_cropSelection.bottomRight());

    // Clamp to image bounds
    topLeft.setX(qBound(0, topLeft.x(), m_imageSize.width()));
    topLeft.setY(qBound(0, topLeft.y(), m_imageSize.height()));
    bottomRight.setX(qBound(0, bottomRight.x(), m_imageSize.width()));
    bottomRight.setY(qBound(0, bottomRight.y(), m_imageSize.height()));

    return QRect(topLeft, bottomRight).normalized();
}

void CanvasWidget::applyCrop() {
    QRect cropArea = getCropSelection();
    if (cropArea.isValid() && !cropArea.isEmpty()) {
        Q_EMIT cropRequested(cropArea);
    }
    setCropMode(false);
}

void CanvasWidget::cancelCrop() {
    setCropMode(false);
}

QPoint CanvasWidget::widgetToImageCoords(const QPoint& widgetPos) const {
    if (m_imageSize.isEmpty()) {
        return QPoint();
    }

    // Calculate the image display area
    double widgetAspect = static_cast<double>(width()) / height();
    double imageAspect = static_cast<double>(m_imageSize.width()) / m_imageSize.height();

    double displayW, displayH;
    double offsetX, offsetY;

    if (imageAspect > widgetAspect) {
        // Image is wider - fit to width
        displayW = width() * m_zoomFactor;
        displayH = displayW / imageAspect;
    } else {
        // Image is taller - fit to height
        displayH = height() * m_zoomFactor;
        displayW = displayH * imageAspect;
    }

    offsetX = (width() - displayW) / 2.0 + m_panOffset.x() * width() * m_zoomFactor;
    offsetY = (height() - displayH) / 2.0 - m_panOffset.y() * height() * m_zoomFactor;

    // Convert widget position to image position
    double imageX = (widgetPos.x() - offsetX) / displayW * m_imageSize.width();
    double imageY = (widgetPos.y() - offsetY) / displayH * m_imageSize.height();

    return QPoint(static_cast<int>(imageX), static_cast<int>(imageY));
}

QPoint CanvasWidget::imageToWidgetCoords(const QPoint& imagePos) const {
    if (m_imageSize.isEmpty()) {
        return QPoint();
    }

    double widgetAspect = static_cast<double>(width()) / height();
    double imageAspect = static_cast<double>(m_imageSize.width()) / m_imageSize.height();

    double displayW, displayH;
    double offsetX, offsetY;

    if (imageAspect > widgetAspect) {
        displayW = width() * m_zoomFactor;
        displayH = displayW / imageAspect;
    } else {
        displayH = height() * m_zoomFactor;
        displayW = displayH * imageAspect;
    }

    offsetX = (width() - displayW) / 2.0 + m_panOffset.x() * width() * m_zoomFactor;
    offsetY = (height() - displayH) / 2.0 - m_panOffset.y() * height() * m_zoomFactor;

    double widgetX = static_cast<double>(imagePos.x()) / m_imageSize.width() * displayW + offsetX;
    double widgetY = static_cast<double>(imagePos.y()) / m_imageSize.height() * displayH + offsetY;

    return QPoint(static_cast<int>(widgetX), static_cast<int>(widgetY));
}

void CanvasWidget::drawCropOverlay(QPainter& painter) {
    if (!m_cropMode)
        return;

    // Draw semi-transparent overlay outside selection
    painter.fillRect(rect(), QColor(0, 0, 0, 100));

    if (!m_cropSelection.isEmpty()) {
        // Clear the selection area (show the image)
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(m_cropSelection, Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // Draw selection border
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter.drawRect(m_cropSelection);

        // Draw corner handles
        const int handleSize = 8;
        painter.setBrush(Qt::white);
        painter.setPen(Qt::black);

        QRect tl(m_cropSelection.topLeft() - QPoint(handleSize / 2, handleSize / 2),
                 QSize(handleSize, handleSize));
        QRect tr(m_cropSelection.topRight() - QPoint(handleSize / 2, handleSize / 2),
                 QSize(handleSize, handleSize));
        QRect bl(m_cropSelection.bottomLeft() - QPoint(handleSize / 2, handleSize / 2),
                 QSize(handleSize, handleSize));
        QRect br(m_cropSelection.bottomRight() - QPoint(handleSize / 2, handleSize / 2),
                 QSize(handleSize, handleSize));

        painter.drawRect(tl);
        painter.drawRect(tr);
        painter.drawRect(bl);
        painter.drawRect(br);

        // Draw size info
        QRect imageRect = getCropSelection();
        if (imageRect.isValid()) {
            QString sizeText = QString("%1 x %2").arg(imageRect.width()).arg(imageRect.height());
            painter.setPen(Qt::white);
            painter.drawText(m_cropSelection.adjusted(5, 5, 0, 0), sizeText);
        }
    }
}
