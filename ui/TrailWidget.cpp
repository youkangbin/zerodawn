#include "TrailWidget.hpp"
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>
#include <QTimer>

TrailWidget::TrailWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_program(nullptr)
    , m_quadVBO(QOpenGLBuffer::VertexBuffer)
    , m_pointsSSBO(QOpenGLBuffer::VertexBuffer)
    , m_metadataSSBO(QOpenGLBuffer::VertexBuffer)
    , m_gradientTexture(0)
    , m_totalPoints(0)
    , m_cameraDistance(15.0f)
    , m_cameraYaw(45.0f)
    , m_cameraPitch(30.0f)
    , m_mousePressed(false)
    , m_frameCount(0)
    , m_currentFPS(0.0f)
    , m_lastFpsUpdate(0)
{
    m_cameraPos = QVector3D(0, 5, 10);
    m_fpsTimer.start();
    m_lastFpsUpdate = m_fpsTimer.elapsed();
    
    // 60 FPS刷新
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&TrailWidget::update));
    timer->start(16);
}

TrailWidget::~TrailWidget()
{
    makeCurrent();
    delete m_program;
    m_vao.destroy();
    m_quadVBO.destroy();
    m_pointsSSBO.destroy();
    m_metadataSSBO.destroy();
    if (m_gradientTexture) {
        glDeleteTextures(1, &m_gradientTexture);
    }
    doneCurrent();
}

QVector4D TrailWidget::randomColor()
{
    // 生成鲜艳的颜色
    QRandomGenerator *rng = QRandomGenerator::global();
    
    float hue = rng->bounded(360.0f);
    float saturation = 0.7f + rng->bounded(0.3f);
    float value = 0.8f + rng->bounded(0.2f);
    
    // HSV to RGB
    float c = value * saturation;
    float x = c * (1.0f - qAbs(fmod(hue / 60.0f, 2.0f) - 1.0f));
    float m = value - c;
    
    float r, g, b;
    if (hue < 60) {
        r = c; g = x; b = 0;
    } else if (hue < 120) {
        r = x; g = c; b = 0;
    } else if (hue < 180) {
        r = 0; g = c; b = x;
    } else if (hue < 240) {
        r = 0; g = x; b = c;
    } else if (hue < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    return QVector4D(r + m, g + m, b + m, 1.0f);
}

void TrailWidget::addTrail(int pointCount, float radius, float width)
{
    Trail newTrail;
    newTrail.startIndex = m_totalPoints;
    newTrail.pointCount = pointCount;
    newTrail.color = randomColor();
    newTrail.width = width;
    newTrail.glowIntensity = 1.5f + QRandomGenerator::global()->bounded(1.5f);
    newTrail.radius = radius;
    newTrail.heightScale = 1.0f + QRandomGenerator::global()->bounded(2.0f);
    newTrail.cycles = 2.0f + QRandomGenerator::global()->bounded(4.0f);
    
    // 生成轨道数据
    generateTrailData(newTrail, m_totalPoints);
    
    m_trails.push_back(newTrail);
    m_totalPoints += pointCount;
    
    // 更新GPU缓冲区
    if (context()) {
        makeCurrent();
        updateBuffers();
        doneCurrent();
    }
    
    qDebug() << "Added trail:" << m_trails.size() 
             << "| Points:" << pointCount 
             << "| Total points:" << m_totalPoints
             << "| Radius:" << radius
             << "| Width:" << width;
}

void TrailWidget::clearAllTrails()
{
    m_trails.clear();
    m_allTrailPoints.clear();
    m_totalPoints = 0;
    
    if (context()) {
        makeCurrent();
        updateBuffers();
        doneCurrent();
    }
    
    qDebug() << "Cleared all trails";
}

void TrailWidget::generateTrailData(Trail& trail, int globalStartIndex)
{
    const int pointCount = trail.pointCount;
    const float radius = trail.radius;
    const float heightScale = trail.heightScale;
    const float cycles = trail.cycles;
    
    // 随机旋转偏移
    float rotationOffset = QRandomGenerator::global()->bounded(360.0f);
    
    for (int i = 0; i < pointCount; ++i) {
        float t = static_cast<float>(i) / (pointCount - 1);
        float theta = (t * 2.0f * M_PI * cycles) + qDegreesToRadians(rotationOffset);
        
        // 环形基础位置
        float x = radius * qCos(theta);
        float z = radius * qSin(theta);
        
        // y = sin(x, z) 高度调制
        float y = heightScale * qSin(x * 0.5f) * qCos(z * 0.5f);
        
        // timestamp用于淡出动画
        float timestamp = t;
        
        m_allTrailPoints.push_back(QVector4D(x, y, z, timestamp));
    }
}

void TrailWidget::updateBuffers()
{
    if (m_allTrailPoints.empty()) {
        return;
    }
    
    // 更新Points SSBO
    m_pointsSSBO.bind();
    m_pointsSSBO.allocate(m_allTrailPoints.data(), m_allTrailPoints.size() * sizeof(QVector4D));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pointsSSBO.bufferId());
    
    // 更新Metadata SSBO (每条轨道的元数据)
    std::vector<QVector4D> metadata;
    metadata.reserve(m_trails.size() * 2);
    
    for (const auto& trail : m_trails) {
        metadata.push_back(trail.color);
        metadata.push_back(QVector4D(trail.width, trail.glowIntensity, 
                                      trail.startIndex, trail.pointCount));
    }
    
    m_metadataSSBO.bind();
    m_metadataSSBO.allocate(metadata.data(), metadata.size() * sizeof(QVector4D));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_metadataSSBO.bufferId());
}

void TrailWidget::initializeGL()
{
    initializeOpenGLFunctions();
    
    qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
    qDebug() << "GLSL Version:" << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "Renderer:" << (const char*)glGetString(GL_RENDERER);
    
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 编译着色器
    m_program = new QOpenGLShaderProgram(this);
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/trail.vert")) {
        qWarning() << "Vertex shader error:" << m_program->log();
    }
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/trail.frag")) {
        qWarning() << "Fragment shader error:" << m_program->log();
    }
    if (!m_program->link()) {
        qWarning() << "Shader link error:" << m_program->log();
    }
    
    // 创建Quad几何
    float quadVertices[] = {
        -0.5f, 0.0f,
         0.5f, 0.0f,
        -0.5f, 1.0f,
         0.5f, 1.0f
    };
    
    m_vao.create();
    m_vao.bind();
    
    m_quadVBO.create();
    m_quadVBO.bind();
    m_quadVBO.allocate(quadVertices, sizeof(quadVertices));
    
    m_program->bind();
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    
    // 创建空的SSBO
    m_pointsSSBO.create();
    m_pointsSSBO.bind();
    m_pointsSSBO.allocate(nullptr, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pointsSSBO.bufferId());
    
    m_metadataSSBO.create();
    m_metadataSSBO.bind();
    m_metadataSSBO.allocate(nullptr, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_metadataSSBO.bufferId());
    
    // 创建渐变纹理
    createGradientTexture();
    
    m_vao.release();
    m_program->release();
}

void TrailWidget::createGradientTexture()
{
    const int texSize = 256;
    std::vector<float> gradient(texSize);
    
    for (int i = 0; i < texSize; ++i) {
        float x = static_cast<float>(i) / (texSize - 1);
        float edge = 1.0f - qAbs(x * 2.0f - 1.0f);
        gradient[i] = qPow(edge, 2.0f);
    }
    
    glGenTextures(1, &m_gradientTexture);
    glBindTexture(GL_TEXTURE_1D, m_gradientTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, texSize, 0, GL_RED, GL_FLOAT, gradient.data());
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

void TrailWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, float(w) / float(h), 0.1f, 1000.0f);
}

void TrailWidget::updateCamera()
{
    float radYaw = qDegreesToRadians(m_cameraYaw);
    float radPitch = qDegreesToRadians(m_cameraPitch);
    
    m_cameraPos.setX(m_cameraDistance * qCos(radPitch) * qCos(radYaw));
    m_cameraPos.setY(m_cameraDistance * qSin(radPitch));
    m_cameraPos.setZ(m_cameraDistance * qCos(radPitch) * qSin(radYaw));
    
    m_view.setToIdentity();
    m_view.lookAt(m_cameraPos, QVector3D(0, 0, 0), QVector3D(0, 1, 0));
}

void TrailWidget::paintGL()
{
    // FPS统计 (每0.5秒更新一次)
    m_frameCount++;
    qint64 currentTime = m_fpsTimer.elapsed();
    if (currentTime - m_lastFpsUpdate >= 500) {
        m_currentFPS = m_frameCount * 1000.0f / (currentTime - m_lastFpsUpdate);
        m_frameCount = 0;
        m_lastFpsUpdate = currentTime;
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (m_trails.empty()) {
        return;
    }
    
    updateCamera();
    
    m_program->bind();
    m_vao.bind();
    
    QMatrix4x4 mvp = m_projection * m_view;
    m_program->setUniformValue("viewProjection", mvp);
    m_program->setUniformValue("cameraPos", m_cameraPos);
    m_program->setUniformValue("currentTime", static_cast<float>(m_fpsTimer.elapsed()) / 1000.0f);
    m_program->setUniformValue("maxAge", 10.0f);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_gradientTexture);
    m_program->setUniformValue("gradientTex", 0);
    
    // 逐轨道绘制
    for (size_t i = 0; i < m_trails.size(); ++i) {
        const Trail& trail = m_trails[i];
        
        m_program->setUniformValue("trailIndex", static_cast<int>(i));
        
        int instanceCount = trail.pointCount - 1;
        if (instanceCount > 0) {
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
        }
    }
    
    m_vao.release();
    m_program->release();
}

void TrailWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();
    }
}

void TrailWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressed) {
        QPoint delta = event->pos() - m_lastMousePos;
        
        m_cameraYaw += delta.x() * 0.5f;
        m_cameraPitch += delta.y() * 0.5f;
        
        m_cameraPitch = qBound(-89.0f, m_cameraPitch, 89.0f);
        
        m_lastMousePos = event->pos();
    }
}

void TrailWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = false;
    }
}

void TrailWidget::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_cameraDistance -= delta * 0.5f;
    m_cameraDistance = qBound(2.0f, m_cameraDistance, 50.0f);
}

