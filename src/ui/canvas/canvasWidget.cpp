#include "canvasWidget.h"

#include <QDebug>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

CanvasWidget::CanvasWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      m_texture(nullptr),
      m_shaderProgram(nullptr),
      m_zoomFactor(1.0),
      m_panOffset(0, 0),
      m_panning(false) {}

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
    updateMatrices();
    update();

    doneCurrent();
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
        emit zoomInRequested();
    } else {
        emit zoomOutRequested();
    }
    event->accept();
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
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
        m_panning = false;
    }
}
