#include "canvasWidget.h"

#include <QDebug>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

CanvasWidget::CanvasWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      texture(nullptr),
      m_shaderProgram(nullptr),
      m_zoomFactor(1.0),
      m_panOffset(0, 0),
      m_panning(false) {}

CanvasWidget::~CanvasWidget() {
    makeCurrent();
    delete texture;
    delete m_shaderProgram;
    doneCurrent();
}

void CanvasWidget::setImage(const QImage& image) {
    if (image.isNull())
        return;

    makeCurrent();

    // Clean up old texture
    delete texture;

    // Create new texture
    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);
    texture = new QOpenGLTexture(rgbaImage);
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

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
    m_shaderProgram = new QOpenGLShaderProgram(this);

    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        uniform mat4 u_mvpMatrix;
        
        void main() {
            gl_Position = u_mvpMatrix * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D u_texture;
        
        void main() {
            FragColor = texture(u_texture, TexCoord);
        }
    )";

    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
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

    // View matrix (zoom and pan)
    m_viewMatrix.setToIdentity();
    m_viewMatrix.scale(m_zoomFactor);
    m_viewMatrix.translate(m_panOffset.x(), m_panOffset.y());

    // Combined MVP matrix
    m_mvpMatrix = m_projectionMatrix * m_viewMatrix;
}

void CanvasWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (!texture || !m_shaderProgram) {
        return;
    }

    // Use shader program
    m_shaderProgram->bind();

    // Bind texture to texture unit 0
    texture->bind(0);
    m_shaderProgram->setUniformValue("u_texture", 0);

    // Set transformation matrices for zoom/pan
    m_shaderProgram->setUniformValue("u_mvpMatrix", m_mvpMatrix);

    // Draw the quad
    m_quadVAO.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_quadVAO.release();

    // Cleanup
    texture->release();
    m_shaderProgram->release();
}

void CanvasWidget::zoomIn() {
    m_zoomFactor = qMin(m_zoomFactor * ZOOM_STEP, MAX_ZOOM);
    updateMatrices();
    update();
    emit zoomChanged(m_zoomFactor);
}

void CanvasWidget::zoomOut() {
    m_zoomFactor = qMax(m_zoomFactor / ZOOM_STEP, MIN_ZOOM);
    updateMatrices();
    update();
    emit zoomChanged(m_zoomFactor);
}

void CanvasWidget::zoomToFit() {
    if (m_imageSize.isEmpty())
        return;

    double widgetAspect = (double)width() / height();
    double imageAspect = (double)m_imageSize.width() / m_imageSize.height();

    if (imageAspect > widgetAspect) {
        m_zoomFactor = (double)width() / m_imageSize.width();
    } else {
        m_zoomFactor = (double)height() / m_imageSize.height();
    }

    m_panOffset = QPointF(0, 0);
    updateMatrices();
    update();
    emit zoomChanged(m_zoomFactor);
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else {
        zoomOut();
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
