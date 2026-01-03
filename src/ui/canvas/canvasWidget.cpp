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
            // Start crop selection (store widget point, will convert to image coords on release)
            m_selecting = true;
            m_cropStartPoint = event->pos();
            m_cropSelection = QRect();  // Clear until release
            update();
        } else {
            // Normal panning
            m_panning = true;
            m_lastPanPoint = event->pos();
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    // Emit mouse coordinates for info panel
    if (!m_imageSize.isEmpty()) {
        QPoint imagePos = widgetToImageCoords(event->pos());
        Q_EMIT mouseCoordinatesChanged(event->pos(), imagePos);
    }

    if (m_cropMode && m_selecting) {
        // Just trigger repaint - we'll calculate rectangle in paintGL
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
            // Convert widget rectangle to image coordinates and store
            QRect widgetRect = QRect(m_cropStartPoint, event->pos()).normalized();
            QPoint imgTopLeft = widgetToImageCoords(widgetRect.topLeft());
            QPoint imgBottomRight = widgetToImageCoords(widgetRect.bottomRight());
            m_cropSelection = QRect(imgTopLeft, imgBottomRight).normalized();
            qDebug() << "Crop selection stored in IMAGE coords:" << m_cropSelection;
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

    // m_cropSelection is already in image coordinates - just clamp to bounds
    QRect result = m_cropSelection;

    // Clamp to image bounds
    int left = qBound(0, result.left(), m_imageSize.width());
    int top = qBound(0, result.top(), m_imageSize.height());
    int right = qBound(0, result.right(), m_imageSize.width());
    int bottom = qBound(0, result.bottom(), m_imageSize.height());

    result = QRect(QPoint(left, top), QPoint(right, bottom)).normalized();

    qDebug() << "getCropSelection - returning image coords:" << result;

    return result;
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

QRectF CanvasWidget::getDisplayedImageBounds() const {
    if (m_imageSize.isEmpty() || width() == 0 || height() == 0) {
        return QRectF();
    }

    // Must match EXACTLY how OpenGL renders the texture
    // OpenGL uses normalized coords (-1 to 1) which map to the full widget

    double widgetAspect = static_cast<double>(width()) / height();
    double imageAspect = static_cast<double>(m_imageSize.width()) / m_imageSize.height();

    // These represent the size in normalized coordinates (-1 to 1) = 2.0 units
    double normalizedWidth = 2.0;
    double normalizedHeight = 2.0;

    // Apply aspect ratio correction (matches model matrix)
    if (imageAspect > widgetAspect) {
        // Image is wider - height is scaled down
        normalizedHeight *= widgetAspect / imageAspect;
    } else {
        // Image is taller - width is scaled down
        normalizedWidth *= imageAspect / widgetAspect;
    }

    // Apply zoom (matches view matrix scale)
    normalizedWidth *= m_zoomFactor;
    normalizedHeight *= m_zoomFactor;

    // Apply pan (matches view matrix translate - in normalized coords)
    double normalizedCenterX = m_panOffset.x();
    double normalizedCenterY = m_panOffset.y();

    // Convert from normalized coordinates (-1 to 1) to widget pixel coordinates
    // Center is at (0, 0) in normalized space = (width/2, height/2) in pixels
    double pixelWidth = normalizedWidth * width() / 2.0;
    double pixelHeight = normalizedHeight * height() / 2.0;

    double centerX = width() / 2.0 + normalizedCenterX * width() / 2.0;
    double centerY = height() / 2.0 - normalizedCenterY * height() / 2.0;  // Y is inverted

    double offsetX = centerX - pixelWidth / 2.0;
    double offsetY = centerY - pixelHeight / 2.0;

    return QRectF(offsetX, offsetY, pixelWidth, pixelHeight);
}

QPoint CanvasWidget::widgetToImageCoords(const QPoint& widgetPos) const {
    if (m_imageSize.isEmpty()) {
        return QPoint();
    }

    QRectF imageBounds = getDisplayedImageBounds();
    if (imageBounds.isEmpty()) {
        return QPoint();
    }

    // Convert widget position to image position (no X-mirroring needed - display handles flip)
    double imageX = (widgetPos.x() - imageBounds.x()) / imageBounds.width() * m_imageSize.width();
    double imageY = (widgetPos.y() - imageBounds.y()) / imageBounds.height() * m_imageSize.height();

    qDebug() << "widgetToImageCoords:" << widgetPos
             << "-> imagePos:" << QPoint(qRound(imageX), qRound(imageY))
             << "imageBounds:" << imageBounds;

    return QPoint(qRound(imageX), qRound(imageY));
}

QPoint CanvasWidget::imageToWidgetCoords(const QPoint& imagePos) const {
    if (m_imageSize.isEmpty()) {
        return QPoint();
    }

    QRectF imageBounds = getDisplayedImageBounds();
    if (imageBounds.isEmpty()) {
        return QPoint();
    }

    // Convert image position to widget position (no X-mirroring needed - display handles flip)
    double widgetX = static_cast<double>(imagePos.x()) / m_imageSize.width() * imageBounds.width() +
                     imageBounds.x();
    double widgetY =
        static_cast<double>(imagePos.y()) / m_imageSize.height() * imageBounds.height() +
        imageBounds.y();
    return QPoint(qRound(widgetX), qRound(widgetY));
}

void CanvasWidget::drawCropOverlay(QPainter& painter) {
    if (!m_cropMode)
        return;

    // Get the displayed image bounds
    QRectF imageBounds = getDisplayedImageBounds();

    QRect displaySelection;

    // Determine what to display
    if (m_selecting) {
        // User is currently dragging - show live widget rectangle
        QPoint currentPos = mapFromGlobal(QCursor::pos());
        displaySelection = QRect(m_cropStartPoint, currentPos).normalized();
    } else if (!m_cropSelection.isEmpty()) {
        // User has finished selecting - convert stored image coords to widget coords
        QPoint widgetTopLeft = imageToWidgetCoords(m_cropSelection.topLeft());
        QPoint widgetBottomRight = imageToWidgetCoords(m_cropSelection.bottomRight());
        displaySelection = QRect(widgetTopLeft, widgetBottomRight).normalized();
    } else {
        // No selection yet - just darken the entire image slightly
        painter.fillRect(rect(), QColor(0, 0, 0, 80));

        // Draw instruction text
        QString instructions = "Drag to select • Enter to crop • Esc to cancel";
        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);

        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(instructions);
        textRect.adjust(-12, -8, 12, 8);
        textRect.moveCenter(QPoint(width() / 2, 30));

        painter.setBrush(QColor(0, 0, 0, 200));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(textRect, 5, 5);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, instructions);
        return;
    }

    // Draw semi-transparent overlay everywhere EXCEPT the selection
    QRegion overlayRegion(rect());
    overlayRegion = overlayRegion.subtracted(QRegion(displaySelection));
    painter.setClipRegion(overlayRegion);
    painter.fillRect(rect(), QColor(0, 0, 0, 120));
    painter.setClipping(false);

    // Draw selection border with glow effect
    painter.setPen(QPen(QColor(50, 50, 50, 180), 3));
    painter.drawRect(displaySelection.adjusted(-1, -1, 1, 1));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(displaySelection);

    // Draw rule of thirds grid
    if (m_showRuleOfThirds && displaySelection.width() > 60 && displaySelection.height() > 60) {
        painter.setPen(QPen(QColor(255, 255, 255, 120), 1, Qt::DotLine));
        int w = displaySelection.width();
        int h = displaySelection.height();
        int x = displaySelection.x();
        int y = displaySelection.y();

        // Vertical thirds
        painter.drawLine(x + w / 3, y, x + w / 3, y + h);
        painter.drawLine(x + 2 * w / 3, y, x + 2 * w / 3, y + h);

        // Horizontal thirds
        painter.drawLine(x, y + h / 3, x + w, y + h / 3);
        painter.drawLine(x, y + 2 * h / 3, x + w, y + 2 * h / 3);
    }

    // Draw corner handles with improved styling
    const int handleSize = 10;
    painter.setBrush(Qt::white);
    painter.setPen(QPen(QColor(50, 50, 50), 2));

    QRect tl(displaySelection.topLeft() - QPoint(handleSize / 2, handleSize / 2),
             QSize(handleSize, handleSize));
    QRect tr(displaySelection.topRight() - QPoint(handleSize / 2, handleSize / 2),
             QSize(handleSize, handleSize));
    QRect bl(displaySelection.bottomLeft() - QPoint(handleSize / 2, handleSize / 2),
             QSize(handleSize, handleSize));
    QRect br(displaySelection.bottomRight() - QPoint(handleSize / 2, handleSize / 2),
             QSize(handleSize, handleSize));

    painter.drawEllipse(tl);
    painter.drawEllipse(tr);
    painter.drawEllipse(bl);
    painter.drawEllipse(br);

    // Draw size info
    QRect dimensionRect;
    if (m_selecting) {
        // During drag, convert current widget rect to image coords for display
        QPoint imgTL = widgetToImageCoords(displaySelection.topLeft());
        QPoint imgBR = widgetToImageCoords(displaySelection.bottomRight());
        dimensionRect = QRect(imgTL, imgBR).normalized();
    } else {
        // After selection, use stored image coords directly
        dimensionRect = m_cropSelection;
    }

    if (dimensionRect.isValid()) {
        QString sizeText =
            QString("%1 × %2 px").arg(dimensionRect.width()).arg(dimensionRect.height());
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);

        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(sizeText);
        textRect.adjust(-6, -3, 6, 3);
        textRect.moveTo(displaySelection.left() + 8, displaySelection.top() + 8);

        painter.setBrush(QColor(0, 0, 0, 180));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(textRect, 3, 3);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, sizeText);
    }

    // Draw instruction text when selection is high enough
    if (displaySelection.isEmpty() || displaySelection.top() > 60) {
        QString instructions = "Drag to select • Enter to crop • Esc to cancel";
        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);

        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(instructions);
        textRect.adjust(-12, -8, 12, 8);
        textRect.moveCenter(QPoint(width() / 2, 30));

        painter.setBrush(QColor(0, 0, 0, 200));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(textRect, 5, 5);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, instructions);
    }
}
