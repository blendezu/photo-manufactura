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
      m_panning(false),
      m_zoomMode(ZoomMode::None) {
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

    // Check if dimensions changed (need to reset view)
    bool dimensionsChanged = (m_imageSize != image.size());

    makeCurrent();

    // Clean up old texture
    m_texture.reset();

    // Create new texture
    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);
    m_texture = std::make_unique<QOpenGLTexture>(rgbaImage);
    m_texture->setMinificationFilter(QOpenGLTexture::Linear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);

    m_imageSize = image.size();

    // Only reset view when image dimensions change (e.g., new image loaded)
    // This preserves zoom/pan during adjustment updates
    if (dimensionsChanged) {
        resetView();
    }

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
        uniform sampler2D u_textureOriginal;
        uniform bool u_compareMode;
        uniform float u_splitPosition;
        
        void main() {
            if (u_compareMode && TexCoord.x < u_splitPosition) {
                gl_FragColor = texture2D(u_textureOriginal, TexCoord);
            } else {
                gl_FragColor = texture2D(u_texture, TexCoord);
            }
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

    // Bind original texture for comparison
    if (m_compareMode && m_originalTexture) {
        m_originalTexture->bind(1);
        m_shaderProgram->setUniformValue("u_textureOriginal", 1);
        m_shaderProgram->setUniformValue("u_compareMode", true);
        m_shaderProgram->setUniformValue("u_splitPosition", (float)m_compareSplitPosition);
    } else {
        m_shaderProgram->setUniformValue("u_compareMode", false);
    }

    // Set transformation matrices for zoom/pan
    m_shaderProgram->setUniformValue("u_mvpMatrix", m_mvpMatrix);

    // Draw the quad
    m_quadVAO.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_quadVAO.release();

    // Release textures
    if (m_compareMode && m_originalTexture) {
        m_originalTexture->release();
    }

    // Cleanup
    m_texture->release();
    m_shaderProgram->release();

    // Draw compare overlay using QPainter (after OpenGL)
    if (m_compareMode && m_originalTexture) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        drawCompareOverlay(painter);
        painter.end();
    }

    // Draw crop overlay using QPainter (after OpenGL)
    if (m_cropMode) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        if (m_cropType == CropType::FourPoint) {
            drawFourPointOverlay(painter);
        } else {
            drawCropOverlay(painter);
        }
        painter.end();
    }

    // Draw straighten mode crop preview overlay
    if (m_straightenMode && std::abs(m_straightenAngle) > 0.1f) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        drawStraightenOverlay(painter);
        painter.end();
    }
}

void CanvasWidget::setZoomLevel(double level) {
    // Receive zoom level from controller and update rendering
    if (m_zoomFactor != level) {
        m_zoomFactor = level;
        updateMatrices();
        update();
        Q_EMIT zoomLevelChanged(level);
    }
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    // Only zoom with mouse wheel when zoom mode is enabled
    if (m_zoomMode == ZoomMode::None) {
        event->ignore();
        return;
    }

    // Natural scroll behavior: scroll up = zoom in, scroll down = zoom out
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

    // Space toggles compare mode
    if (event->key() == Qt::Key_Space && !m_cropMode) {
        toggleCompareMode();
        event->accept();
        return;
    }

    QOpenGLWidget::keyPressEvent(event);
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Check if clicking on compare split divider
        if (m_compareMode) {
            QRectF imageBounds = getDisplayedImageBounds();
            int splitX =
                static_cast<int>(imageBounds.left() + imageBounds.width() * m_compareSplitPosition);
            if (qAbs(event->pos().x() - splitX) < 20) {
                m_draggingSplit = true;
                setCursor(Qt::SplitHCursor);
                return;
            }
        }

        // Handle zoom mode clicks: Click = zoom in, Alt+Click = zoom out
        if (m_zoomMode == ZoomMode::Zoom) {
            if (event->modifiers() & Qt::AltModifier) {
                Q_EMIT zoomOutRequested();
            } else {
                Q_EMIT zoomInRequested();
            }
            return;
        }

        if (m_cropMode) {
            // Four-point mode: check if clicking on a corner handle
            if (m_cropType == CropType::FourPoint) {
                int corner = hitTestCorner(event->pos());
                if (corner >= 0) {
                    m_draggedCorner = corner;
                    setCursor(Qt::SizeAllCursor);
                    update();
                    return;
                }
            }

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
    // Handle compare mode split dragging
    if (m_draggingSplit && m_compareMode) {
        QRectF imageBounds = getDisplayedImageBounds();
        double newPosition = (event->pos().x() - imageBounds.left()) / imageBounds.width();
        setCompareSplitPosition(newPosition);
        return;
    }

    // Update cursor when near split divider
    if (m_compareMode && !m_cropMode) {
        QRectF imageBounds = getDisplayedImageBounds();
        int splitX =
            static_cast<int>(imageBounds.left() + imageBounds.width() * m_compareSplitPosition);
        if (qAbs(event->pos().x() - splitX) < 20) {
            setCursor(Qt::SplitHCursor);
        } else if (!m_panning) {
            setCursor(Qt::ArrowCursor);
        }
    }

    if (m_cropMode && m_selecting) {
        // Just trigger repaint - we'll calculate rectangle in paintGL
        update();
    } else if (m_cropMode && m_draggedCorner >= 0) {
        // Four-point mode: dragging a corner
        QPointF newRatio = widgetToFourPointRatio(event->pos());
        // Clamp to valid range
        newRatio.setX(qBound(0.0, newRatio.x(), 1.0));
        newRatio.setY(qBound(0.0, newRatio.y(), 1.0));
        m_fourPointQuad[m_draggedCorner] = newRatio;
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
        // Handle compare split drag release
        if (m_draggingSplit) {
            m_draggingSplit = false;
            setCursor(Qt::ArrowCursor);
            return;
        }

        // Handle four-point corner drag release
        if (m_cropMode && m_draggedCorner >= 0) {
            m_draggedCorner = -1;
            setCursor(Qt::CrossCursor);
            qDebug() << "Four-point quad updated:"
                     << "TL=" << m_fourPointQuad.topLeft << "TR=" << m_fourPointQuad.topRight
                     << "BR=" << m_fourPointQuad.bottomRight << "BL=" << m_fourPointQuad.bottomLeft;
            update();
            return;
        }

        if (m_cropMode && m_selecting) {
            m_selecting = false;
            // Use constrained crop selection based on crop type
            m_cropSelection = constrainCropSelection(m_cropStartPoint, event->pos());
            qDebug() << "Crop selection stored in IMAGE coords:" << m_cropSelection
                     << "CropType:" << static_cast<int>(m_cropType);
            update();
        } else {
            m_panning = false;
        }
    }
}

// Zoom mode methods
void CanvasWidget::setZoomMode(ZoomMode mode) {
    if (m_zoomMode != mode) {
        m_zoomMode = mode;

        // Update cursor based on mode
        if (mode == ZoomMode::Zoom) {
            setCursor(Qt::CrossCursor);  // Zoom cursor
        } else {
            setCursor(Qt::ArrowCursor);
        }

        // Disable crop mode when zoom mode is enabled
        if (mode != ZoomMode::None && m_cropMode) {
            setCropMode(false);
        }

        Q_EMIT zoomModeChanged(mode);
        update();
    }
}

// Crop mode methods
void CanvasWidget::setCropMode(bool enabled) {
    if (m_cropMode != enabled) {
        m_cropMode = enabled;
        m_selecting = false;
        m_cropSelection = QRect();
        m_draggedCorner = -1;  // Reset four-point drag state

        // Disable zoom mode when crop mode is enabled
        if (enabled && m_zoomMode != ZoomMode::None) {
            m_zoomMode = ZoomMode::None;
            Q_EMIT zoomModeChanged(ZoomMode::None);
        }

        // Initialize four-point quad when entering four-point mode
        if (enabled && m_cropType == CropType::FourPoint) {
            m_fourPointQuad.reset();  // Start with full image rectangle
        }

        setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
        if (enabled) {
            setFocus();  // Grab keyboard focus for Enter/Escape
        }
        Q_EMIT cropModeChanged(enabled);
        update();
    }
}

void CanvasWidget::setCropType(CropType type) {
    if (m_cropType != type) {
        m_cropType = type;
        m_cropSelection = QRect();  // Reset selection when type changes
        m_draggedCorner = -1;       // Reset four-point drag state

        // Update aspect ratio based on type
        if (type == CropType::Free) {
            m_aspectRatio = 0.0;
        } else if (type == CropType::FixedSize && m_fixedCropSize.isValid()) {
            m_aspectRatio = static_cast<double>(m_fixedCropSize.width()) / m_fixedCropSize.height();
        } else if (type == CropType::FourPoint) {
            m_fourPointQuad.reset();  // Reset to full image rectangle
        }

        Q_EMIT cropTypeChanged(type);
        update();
    }
}

void CanvasWidget::setFixedCropSize(const QSize& size) {
    if (size.isValid() && size.width() > 0 && size.height() > 0) {
        m_fixedCropSize = size;
        if (m_cropType == CropType::FixedSize) {
            m_aspectRatio = static_cast<double>(size.width()) / size.height();
            m_cropSelection = QRect();  // Reset selection
            update();
        }
    }
}

void CanvasWidget::setAspectRatioPreset(AspectRatioPreset preset) {
    m_aspectPreset = preset;
    m_aspectRatio = getPresetAspectRatio(preset);

    if (preset == AspectRatioPreset::Free) {
        m_cropType = CropType::Free;
    } else {
        m_cropType = CropType::AspectRatio;
    }

    m_cropSelection = QRect();  // Reset selection
    Q_EMIT cropTypeChanged(m_cropType);
    update();
}

void CanvasWidget::setCustomAspectRatio(double widthRatio, double heightRatio) {
    if (widthRatio > 0 && heightRatio > 0) {
        m_aspectRatio = widthRatio / heightRatio;
        m_aspectPreset = AspectRatioPreset::Custom;
        m_cropType = CropType::AspectRatio;
        m_cropSelection = QRect();
        Q_EMIT cropTypeChanged(m_cropType);
        update();
    }
}

// ============================================================================
// Straighten Mode Methods
// ============================================================================

void CanvasWidget::setStraightenMode(bool enabled) {
    if (m_straightenMode != enabled) {
        m_straightenMode = enabled;
        m_straightenAngle = 0.0f;

        // Disable other modes
        if (enabled) {
            if (m_cropMode)
                setCropMode(false);
            if (m_zoomMode != ZoomMode::None)
                setZoomMode(ZoomMode::None);
        }

        setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
        Q_EMIT straightenModeChanged(enabled);
        update();
    }
}

void CanvasWidget::setStraightenAngle(float angle) {
    if (m_straightenAngle != angle) {
        m_straightenAngle = angle;
        update();
    }
}

QRect CanvasWidget::getInscribedCropRect() const {
    if (m_imageSize.isEmpty() || std::abs(m_straightenAngle) < 0.001f) {
        return QRect(0, 0, m_imageSize.width(), m_imageSize.height());
    }

    // Calculate largest inscribed rectangle after rotation
    // Formula: For rotation angle θ, the inscribed rectangle dimensions are:
    // newW = W * cos(θ) - H * sin(θ) (for W > H after accounting for aspect)
    // This is the conservative crop that removes all black corners

    double W = m_imageSize.width();
    double H = m_imageSize.height();
    double theta = std::abs(m_straightenAngle) * M_PI / 180.0;

    double sinA = std::sin(theta);
    double cosA = std::cos(theta);

    // For a rotated rectangle, the largest inscribed axis-aligned rectangle:
    // If aspect = W/H, the inscribed rectangle has:
    double aspect = W / H;

    double newW, newH;
    if (aspect >= 1.0) {
        // Landscape: width constrained
        newW = W * cosA - H * sinA;
        newH = H * cosA - W * sinA;
    } else {
        // Portrait: height constrained
        newW = W * cosA - H * sinA;
        newH = H * cosA - W * sinA;
    }

    // Ensure positive dimensions
    newW = std::max(10.0, newW);
    newH = std::max(10.0, newH);

    // Center the crop rectangle
    int left = static_cast<int>((W - newW) / 2);
    int top = static_cast<int>((H - newH) / 2);

    return QRect(left, top, static_cast<int>(newW), static_cast<int>(newH));
}

double CanvasWidget::getPresetAspectRatio(AspectRatioPreset preset) const {
    switch (preset) {
        case AspectRatioPreset::Square_1_1:
            return 1.0;
        case AspectRatioPreset::Photo_4_3:
            return 4.0 / 3.0;
        case AspectRatioPreset::Photo_3_2:
            return 3.0 / 2.0;
        case AspectRatioPreset::Widescreen_16_9:
            return 16.0 / 9.0;
        case AspectRatioPreset::Widescreen_21_9:
            return 21.0 / 9.0;
        case AspectRatioPreset::Portrait_3_4:
            return 3.0 / 4.0;
        case AspectRatioPreset::Portrait_2_3:
            return 2.0 / 3.0;
        case AspectRatioPreset::Portrait_9_16:
            return 9.0 / 16.0;
        case AspectRatioPreset::Custom:
        case AspectRatioPreset::Free:
        default:
            return 0.0;  // 0 means free/no constraint
    }
}

QString CanvasWidget::getCropModeLabel() const {
    switch (m_cropType) {
        case CropType::FourPoint:
            return "4-Point Perspective";
        case CropType::FixedSize:
            return QString("Fixed: %1×%2 px")
                .arg(m_fixedCropSize.width())
                .arg(m_fixedCropSize.height());
        case CropType::AspectRatio:
            switch (m_aspectPreset) {
                case AspectRatioPreset::Square_1_1:
                    return "1:1 Square";
                case AspectRatioPreset::Photo_4_3:
                    return "4:3 Photo";
                case AspectRatioPreset::Photo_3_2:
                    return "3:2 Photo";
                case AspectRatioPreset::Widescreen_16_9:
                    return "16:9 Widescreen";
                case AspectRatioPreset::Widescreen_21_9:
                    return "21:9 Ultrawide";
                case AspectRatioPreset::Portrait_3_4:
                    return "3:4 Portrait";
                case AspectRatioPreset::Portrait_2_3:
                    return "2:3 Portrait";
                case AspectRatioPreset::Portrait_9_16:
                    return "9:16 Portrait";
                case AspectRatioPreset::Custom:
                    return QString("Custom %1").arg(m_aspectRatio, 0, 'f', 2);
                default:
                    return "Aspect Ratio";
            }
        case CropType::Free:
        default:
            return "Free";
    }
}

QRect CanvasWidget::constrainCropSelection(const QPoint& startWidget,
                                           const QPoint& endWidget) const {
    // Convert widget points to image coordinates
    QPoint imgStart = widgetToImageCoords(startWidget);
    QPoint imgEnd = widgetToImageCoords(endWidget);

    // Create base rectangle in image coordinates
    QRect baseRect = QRect(imgStart, imgEnd).normalized();

    // Clamp to image bounds first
    baseRect = baseRect.intersected(QRect(QPoint(0, 0), m_imageSize));

    if (baseRect.isEmpty()) {
        return QRect();
    }

    // Apply constraints based on crop type
    switch (m_cropType) {
        case CropType::FixedSize: {
            // Fixed size: center the fixed rectangle at the drag center
            int centerX = (imgStart.x() + imgEnd.x()) / 2;
            int centerY = (imgStart.y() + imgEnd.y()) / 2;

            int halfW = m_fixedCropSize.width() / 2;
            int halfH = m_fixedCropSize.height() / 2;

            int left = qBound(0, centerX - halfW, m_imageSize.width() - m_fixedCropSize.width());
            int top = qBound(0, centerY - halfH, m_imageSize.height() - m_fixedCropSize.height());

            // Ensure fixed size doesn't exceed image
            int w = qMin(m_fixedCropSize.width(), m_imageSize.width());
            int h = qMin(m_fixedCropSize.height(), m_imageSize.height());

            return QRect(left, top, w, h);
        }

        case CropType::AspectRatio: {
            if (m_aspectRatio <= 0) {
                return baseRect;  // No constraint
            }

            // Maintain aspect ratio - use the smaller dimension to fit
            double currentRatio = static_cast<double>(baseRect.width()) / baseRect.height();
            int newW, newH;

            if (currentRatio > m_aspectRatio) {
                // Too wide, constrain width
                newH = baseRect.height();
                newW = static_cast<int>(newH * m_aspectRatio);
            } else {
                // Too tall, constrain height
                newW = baseRect.width();
                newH = static_cast<int>(newW / m_aspectRatio);
            }

            // Keep the same top-left corner
            QRect constrained(baseRect.topLeft(), QSize(newW, newH));

            // Clamp to image bounds
            if (constrained.right() >= m_imageSize.width()) {
                constrained.moveRight(m_imageSize.width() - 1);
            }
            if (constrained.bottom() >= m_imageSize.height()) {
                constrained.moveBottom(m_imageSize.height() - 1);
            }

            return constrained;
        }

        case CropType::Free:
        default:
            return baseRect;
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
    if (m_cropType == CropType::FourPoint) {
        // Emit four-point perspective crop signal
        Q_EMIT perspectiveCropRequested(m_fourPointQuad);
        qDebug() << "Four-point crop applied:"
                 << "TL=" << m_fourPointQuad.topLeft << "TR=" << m_fourPointQuad.topRight
                 << "BR=" << m_fourPointQuad.bottomRight << "BL=" << m_fourPointQuad.bottomLeft;
    } else {
        // Regular rectangular crop
        QRect cropArea = getCropSelection();
        if (cropArea.isValid() && !cropArea.isEmpty()) {
            Q_EMIT cropRequested(cropArea);
        }
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
    // OpenGL vertex shader: gl_Position = mvpMatrix * position
    // mvpMatrix = projection * view * model
    //
    // Model: scales quad for aspect ratio (quad is -1 to 1)
    // View: scale(zoom) then translate(pan) - so pan is scaled by zoom
    // Projection: ortho(-1,1,-1,1,-1,1)

    double widgetAspect = static_cast<double>(width()) / height();
    double imageAspect = static_cast<double>(m_imageSize.width()) / m_imageSize.height();

    // Start with the image quad size in normalized coordinates
    // The quad is from -1 to 1, so total size is 2.0 units
    double quadWidth = 2.0;
    double quadHeight = 2.0;

    // Apply model matrix: aspect ratio correction
    if (imageAspect > widgetAspect) {
        // Image is wider - height is scaled down
        quadHeight *= widgetAspect / imageAspect;
    } else {
        // Image is taller - width is scaled down
        quadWidth *= imageAspect / widgetAspect;
    }

    // Apply view matrix: first scale, then translate
    // m_viewMatrix.scale(m_zoomFactor);
    // m_viewMatrix.translate(m_panOffset.x(), m_panOffset.y());
    // In Qt matrix ops: V = I * Scale * Translate
    // Transform: V * p = Scale * (Translate * p) = Scale * (p + pan)
    // So: result = zoom * (original + pan) = zoom*original + zoom*pan

    double scaledWidth = quadWidth * m_zoomFactor;
    double scaledHeight = quadHeight * m_zoomFactor;

    // Pan offset is also scaled by zoom (applied before scale in vertex transform)
    double panPixelX = m_panOffset.x() * m_zoomFactor * width() / 2.0;
    double panPixelY = m_panOffset.y() * m_zoomFactor * height() / 2.0;

    // Convert from normalized coordinates to widget pixel coordinates
    // Normalized (-1 to 1) maps to (0 to width) and (0 to height)
    double pixelWidth = scaledWidth * width() / 2.0;
    double pixelHeight = scaledHeight * height() / 2.0;

    // Center of image in widget coordinates
    // Without pan: center is at widget center
    // With pan: offset by panPixel (Y inverted because OpenGL Y is up, widget Y is down)
    double centerX = width() / 2.0 + panPixelX;
    double centerY = height() / 2.0 - panPixelY;  // Y inverted

    double offsetX = centerX - pixelWidth / 2.0;
    double offsetY = centerY - pixelHeight / 2.0;

    qDebug() << "getDisplayedImageBounds:"
             << "widget:" << width() << "x" << height() << "image:" << m_imageSize
             << "zoom:" << m_zoomFactor << "pan:" << m_panOffset
             << "result:" << QRectF(offsetX, offsetY, pixelWidth, pixelHeight);

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
    QRect imageSelection;  // In image coordinates for display

    // Determine what to display
    if (m_selecting) {
        // User is currently dragging - compute constrained selection for live preview
        QPoint currentPos = mapFromGlobal(QCursor::pos());
        imageSelection = constrainCropSelection(m_cropStartPoint, currentPos);

        // Convert to widget coords for display
        if (!imageSelection.isEmpty()) {
            QPoint widgetTopLeft = imageToWidgetCoords(imageSelection.topLeft());
            QPoint widgetBottomRight = imageToWidgetCoords(imageSelection.bottomRight());
            displaySelection = QRect(widgetTopLeft, widgetBottomRight).normalized();
        } else {
            // Fall back to raw widget rect if constraint fails
            displaySelection = QRect(m_cropStartPoint, currentPos).normalized();
        }
    } else if (!m_cropSelection.isEmpty()) {
        // User has finished selecting - convert stored image coords to widget coords
        imageSelection = m_cropSelection;
        QPoint widgetTopLeft = imageToWidgetCoords(m_cropSelection.topLeft());
        QPoint widgetBottomRight = imageToWidgetCoords(m_cropSelection.bottomRight());
        displaySelection = QRect(widgetTopLeft, widgetBottomRight).normalized();
    } else {
        // No selection yet - just darken the entire image slightly
        painter.fillRect(rect(), QColor(0, 0, 0, 80));

        // Draw instruction text with crop mode info
        QString modeLabel = getCropModeLabel();
        QString instructions =
            QString("Mode: %1 • Drag to select • Enter to crop • Esc to cancel").arg(modeLabel);
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

        // Draw size info with mode label
        if (imageSelection.isValid()) {
            QString modeLabel = getCropModeLabel();
            QString sizeText = QString("%1 × %2 px • %3")
                                   .arg(imageSelection.width())
                                   .arg(imageSelection.height())
                                   .arg(modeLabel);
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
}

void CanvasWidget::drawStraightenOverlay(QPainter& painter) {
    if (!m_straightenMode || m_imageSize.isEmpty())
        return;

    // Get the inscribed crop rectangle in image coordinates
    QRect cropRect = getInscribedCropRect();

    // Map crop rect corners to widget coordinates
    QPoint topLeft = imageToWidgetCoords(cropRect.topLeft());
    QPoint bottomRight = imageToWidgetCoords(cropRect.bottomRight());
    QRect displaySelection(topLeft, bottomRight);

    // Draw semi-transparent overlay everywhere EXCEPT the selection
    QRegion overlayRegion(rect());
    overlayRegion = overlayRegion.subtracted(QRegion(displaySelection));
    painter.setClipRegion(overlayRegion);
    painter.fillRect(rect(), QColor(0, 0, 0, 160));  // Darker than normal crop
    painter.setClipping(false);

    // Draw selection border
    painter.setPen(QPen(QColor(80, 150, 255), 2));  // Blue accent for straighten mode
    painter.drawRect(displaySelection);

    // Draw grid overlay (always show 3x3 grid for straightening)
    painter.setPen(QPen(QColor(255, 255, 255, 120), 1));
    int w = displaySelection.width();
    int h = displaySelection.height();
    int x = displaySelection.x();
    int y = displaySelection.y();

    // Dense grid for alignment (4x4 or 3x3)
    for (int i = 1; i <= 3; ++i) {
        // Vertical lines
        int xi = x + i * w / 4;
        painter.drawLine(xi, y, xi, y + h);

        // Horizontal lines
        int yi = y + i * h / 4;
        painter.drawLine(x, yi, x + w, yi);
    }

    // Draw instruction overlay
    QString instructions = QString("Straighten: %1° • Auto-crop enabled")
                               .arg(QString::number(m_straightenAngle, 'f', 1));
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);

    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(instructions);
    textRect.adjust(-12, -8, 12, 8);
    textRect.moveCenter(QPoint(width() / 2, height() - 50));  // Bottom center

    painter.setBrush(QColor(0, 0, 0, 200));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 5, 5);

    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, instructions);
}

// Before/After comparison methods
void CanvasWidget::setOriginalImage(const QImage& image) {
    if (image.isNull())
        return;

    makeCurrent();

    m_originalTexture.reset();

    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);
    m_originalTexture = std::make_unique<QOpenGLTexture>(rgbaImage);
    m_originalTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_originalTexture->setMagnificationFilter(QOpenGLTexture::Linear);

    doneCurrent();
    update();
}

void CanvasWidget::setCompareMode(bool enabled) {
    if (m_compareMode != enabled) {
        m_compareMode = enabled;
        Q_EMIT compareModeChanged(enabled);
        update();
    }
}

void CanvasWidget::toggleCompareMode() {
    setCompareMode(!m_compareMode);
}

void CanvasWidget::setCompareSplitPosition(double position) {
    m_compareSplitPosition = qBound(0.0, position, 1.0);
    update();
}

void CanvasWidget::drawCompareOverlay(QPainter& painter) {
    // Get the displayed image bounds
    QRectF imageBounds = getDisplayedImageBounds();
    if (imageBounds.isEmpty())
        return;

    // Calculate split line position
    int splitX =
        static_cast<int>(imageBounds.left() + imageBounds.width() * m_compareSplitPosition);

    // Draw semi-transparent overlay on the "After" side with label
    // The processed image is shown by default, so we show "BEFORE" label on left side

    // Draw split line
    QPen linePen(QColor(255, 255, 255, 200), 2);
    painter.setPen(linePen);
    painter.drawLine(splitX, static_cast<int>(imageBounds.top()), splitX,
                     static_cast<int>(imageBounds.bottom()));

    // Draw drag handle
    QRect handleRect(splitX - 15, height() / 2 - 30, 30, 60);
    painter.setBrush(QColor(99, 102, 241, 220));  // Indigo
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(handleRect, 8, 8);

    // Draw handle grip lines
    painter.setPen(QPen(Qt::white, 2));
    for (int i = -1; i <= 1; i++) {
        int y = height() / 2 + i * 10;
        painter.drawLine(splitX - 6, y, splitX + 6, y);
    }

    // Draw "BEFORE" and "AFTER" labels
    QFont labelFont = painter.font();
    labelFont.setPointSize(11);
    labelFont.setBold(true);
    painter.setFont(labelFont);

    // Before label (left side)
    QRect beforeRect(static_cast<int>(imageBounds.left()) + 10,
                     static_cast<int>(imageBounds.top()) + 10, 70, 24);
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(beforeRect, 4, 4);
    painter.setPen(Qt::white);
    painter.drawText(beforeRect, Qt::AlignCenter, "BEFORE");

    // After label (right side)
    QRect afterRect(static_cast<int>(imageBounds.right()) - 80,
                    static_cast<int>(imageBounds.top()) + 10, 70, 24);
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(afterRect, 4, 4);
    painter.setPen(Qt::white);
    painter.drawText(afterRect, Qt::AlignCenter, "AFTER");

    // Draw instruction at bottom
    QString instructions = "Drag divider to compare • Press Space to toggle";
    QFontMetrics fm(labelFont);
    QRect textRect = fm.boundingRect(instructions);
    textRect.adjust(-12, -6, 12, 6);
    textRect.moveCenter(QPoint(width() / 2, height() - 30));

    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 5, 5);
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, instructions);
}

// ============================================================================
// Four-Point Perspective Crop Methods
// ============================================================================

void CanvasWidget::setFourPointQuad(const FourPointQuad& quad) {
    m_fourPointQuad = quad;
    update();
}

void CanvasWidget::resetFourPointQuad() {
    m_fourPointQuad.reset();
    update();
}

QPoint CanvasWidget::fourPointCornerToWidget(int cornerIndex) const {
    if (cornerIndex < 0 || cornerIndex > 3 || m_imageSize.isEmpty()) {
        return QPoint();
    }

    QPointF ratio = m_fourPointQuad[cornerIndex];
    QPoint imagePos(static_cast<int>(ratio.x() * m_imageSize.width()),
                    static_cast<int>(ratio.y() * m_imageSize.height()));

    return imageToWidgetCoords(imagePos);
}

QPointF CanvasWidget::widgetToFourPointRatio(const QPoint& widgetPos) const {
    if (m_imageSize.isEmpty()) {
        return QPointF(0.5, 0.5);
    }

    QPoint imagePos = widgetToImageCoords(widgetPos);

    return QPointF(static_cast<double>(imagePos.x()) / m_imageSize.width(),
                   static_cast<double>(imagePos.y()) / m_imageSize.height());
}

int CanvasWidget::hitTestCorner(const QPoint& widgetPos) const {
    for (int i = 0; i < 4; ++i) {
        QPoint cornerWidget = fourPointCornerToWidget(i);
        int dx = widgetPos.x() - cornerWidget.x();
        int dy = widgetPos.y() - cornerWidget.y();
        int distSq = dx * dx + dy * dy;

        if (distSq <= CORNER_HIT_RADIUS * CORNER_HIT_RADIUS) {
            return i;
        }
    }
    return -1;
}

void CanvasWidget::drawFourPointOverlay(QPainter& painter) {
    if (!m_cropMode || m_cropType != CropType::FourPoint) {
        return;
    }

    QRectF imageBounds = getDisplayedImageBounds();
    if (imageBounds.isEmpty()) {
        return;
    }

    // Get widget coordinates for all 4 corners
    QPoint corners[4];
    for (int i = 0; i < 4; ++i) {
        corners[i] = fourPointCornerToWidget(i);
    }

    // Draw semi-transparent overlay everywhere EXCEPT inside the quad
    QPolygon quadPoly;
    quadPoly << corners[0] << corners[1] << corners[2] << corners[3];

    QRegion overlayRegion(rect());
    overlayRegion = overlayRegion.subtracted(QRegion(quadPoly));
    painter.setClipRegion(overlayRegion);
    painter.fillRect(rect(), QColor(0, 0, 0, 120));
    painter.setClipping(false);

    // Draw the quadrilateral outline with glow
    painter.setPen(QPen(QColor(50, 50, 50, 180), 4));
    painter.drawPolygon(quadPoly);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawPolygon(quadPoly);

    // Draw grid lines inside the quad (perspective grid)
    painter.setPen(QPen(QColor(255, 255, 255, 80), 1, Qt::DotLine));

    // Draw 2 horizontal lines (thirds)
    for (int i = 1; i <= 2; ++i) {
        double t = i / 3.0;
        QPointF leftPt = corners[0] + t * (corners[3] - corners[0]);
        QPointF rightPt = corners[1] + t * (corners[2] - corners[1]);
        painter.drawLine(leftPt.toPoint(), rightPt.toPoint());
    }

    // Draw 2 vertical lines (thirds)
    for (int i = 1; i <= 2; ++i) {
        double t = i / 3.0;
        QPointF topPt = corners[0] + t * (corners[1] - corners[0]);
        QPointF bottomPt = corners[3] + t * (corners[2] - corners[3]);
        painter.drawLine(topPt.toPoint(), bottomPt.toPoint());
    }

    // Draw corner handles
    const int handleSize = 14;
    const QString cornerLabels[4] = {"TL", "TR", "BR", "BL"};

    for (int i = 0; i < 4; ++i) {
        QRect handleRect(corners[i].x() - handleSize / 2, corners[i].y() - handleSize / 2,
                         handleSize, handleSize);

        // Highlight the dragged corner
        if (i == m_draggedCorner) {
            painter.setBrush(QColor(99, 102, 241));  // Indigo
            painter.setPen(QPen(Qt::white, 3));
        } else {
            painter.setBrush(Qt::white);
            painter.setPen(QPen(QColor(50, 50, 50), 2));
        }

        painter.drawEllipse(handleRect);
    }

    // Draw corner labels near each handle
    QFont labelFont = painter.font();
    labelFont.setPointSize(9);
    labelFont.setBold(true);
    painter.setFont(labelFont);

    for (int i = 0; i < 4; ++i) {
        // Position label offset from corner
        int offsetX = (i == 0 || i == 3) ? -25 : 12;
        int offsetY = (i == 0 || i == 1) ? -8 : 20;

        QRect labelRect(corners[i].x() + offsetX, corners[i].y() + offsetY, 20, 16);
        painter.setPen(Qt::white);
        painter.drawText(labelRect, Qt::AlignCenter, cornerLabels[i]);
    }

    // Draw coordinate display for each corner
    labelFont.setPointSize(8);
    labelFont.setBold(false);
    painter.setFont(labelFont);
    painter.setPen(QColor(200, 200, 200));

    for (int i = 0; i < 4; ++i) {
        QPointF ratio = m_fourPointQuad[i];
        QString coordText = QString("%1, %2").arg(ratio.x(), 0, 'f', 2).arg(ratio.y(), 0, 'f', 2);

        int offsetX = (i == 0 || i == 3) ? -55 : 12;
        int offsetY = (i == 0 || i == 1) ? 8 : -25;

        QRect coordRect(corners[i].x() + offsetX, corners[i].y() + offsetY, 50, 14);
        painter.drawText(coordRect, Qt::AlignCenter, coordText);
    }

    // Draw instruction text
    QString modeLabel = "4-Point Perspective";
    QString instructions =
        QString("Mode: %1 • Drag corners • Enter to apply • Esc to cancel").arg(modeLabel);

    labelFont.setPointSize(12);
    painter.setFont(labelFont);
    QFontMetrics fm(labelFont);
    QRect textRect = fm.boundingRect(instructions);
    textRect.adjust(-12, -8, 12, 8);
    textRect.moveCenter(QPoint(width() / 2, 30));

    painter.setBrush(QColor(0, 0, 0, 200));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 5, 5);

    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, instructions);
}