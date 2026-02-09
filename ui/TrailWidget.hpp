#ifndef TRAILWIDGET_H
#define TRAILWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <vector>

struct Trail {
    int startIndex;      // 在SSBO中的起始索引
    int pointCount;      // 点数
    QVector4D color;     // 颜色
    float width;         // 宽度
    float glowIntensity; // 发光强度
    float radius;        // 轨道半径
    float heightScale;   // 高度缩放
    float cycles;        // 环绕圈数
};

class TrailWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    explicit TrailWidget(QWidget *parent = nullptr);
    ~TrailWidget();
    
    float getCurrentFPS() const { return m_currentFPS; }
    int getTrailCount() const { return m_trails.size(); }
    int getTotalPoints() const { return m_totalPoints; }

public slots:
    void addTrail(int pointCount, float radius, float width);
    void clearAllTrails();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void generateTrailData(Trail& trail, int globalStartIndex);
    void createGradientTexture();
    void updateCamera();
    void updateBuffers();
    QVector4D randomColor();
    
    // OpenGL对象
    QOpenGLShaderProgram *m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_quadVBO;
    QOpenGLBuffer m_pointsSSBO;
    QOpenGLBuffer m_metadataSSBO;
    GLuint m_gradientTexture;
    
    // 多轨道管理
    std::vector<Trail> m_trails;
    std::vector<QVector4D> m_allTrailPoints;  // 所有轨道的点
    int m_totalPoints;
    
    // 相机控制
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QVector3D m_cameraPos;
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;
    
    QPoint m_lastMousePos;
    bool m_mousePressed;
    
    // 性能统计
    QElapsedTimer m_fpsTimer;
    int m_frameCount;
    float m_currentFPS;
    qint64 m_lastFpsUpdate;
};

#endif // TRAILWIDGET_H
