#include "MainWindow.hpp"
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Instanced Quad Trail Demo - Multi Trail");
    resize(1400, 800);
    
    // 创建主分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);
    
    // 左侧：OpenGL渲染窗口
    m_trailWidget = new TrailWidget(this);
    splitter->addWidget(m_trailWidget);
    
    // 右侧：控制面板
    QWidget *controlPanel = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(controlPanel);
    
    // === 轨道参数设置 ===
    QGroupBox *paramGroup = new QGroupBox("Trail Parameters", this);
    QVBoxLayout *paramLayout = new QVBoxLayout(paramGroup);
    
    // 点数
    QHBoxLayout *pointLayout = new QHBoxLayout();
    pointLayout->addWidget(new QLabel("Point Count:", this));
    m_pointCountSpinBox = new QSpinBox(this);
    m_pointCountSpinBox->setRange(100, 20000);
    m_pointCountSpinBox->setValue(5000);
    m_pointCountSpinBox->setSingleStep(100);
    pointLayout->addWidget(m_pointCountSpinBox);
    paramLayout->addLayout(pointLayout);
    
    // 半径
    QHBoxLayout *radiusLayout = new QHBoxLayout();
    radiusLayout->addWidget(new QLabel("Radius:", this));
    m_radiusSpinBox = new QDoubleSpinBox(this);
    m_radiusSpinBox->setRange(1.0, 20.0);
    m_radiusSpinBox->setValue(5.0);
    m_radiusSpinBox->setSingleStep(0.5);
    m_radiusSpinBox->setDecimals(1);
    radiusLayout->addWidget(m_radiusSpinBox);
    paramLayout->addLayout(radiusLayout);
    
    // 宽度
    QHBoxLayout *widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel("Width:", this));
    m_widthSpinBox = new QDoubleSpinBox(this);
    m_widthSpinBox->setRange(0.01, 1.0);
    m_widthSpinBox->setValue(0.08);
    m_widthSpinBox->setSingleStep(0.01);
    m_widthSpinBox->setDecimals(2);
    widthLayout->addWidget(m_widthSpinBox);
    paramLayout->addLayout(widthLayout);
    
    mainLayout->addWidget(paramGroup);
    
    // === 操作按钮 ===
    m_addButton = new QPushButton("Add Trail", this);
    m_addButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; }");
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddTrail);
    mainLayout->addWidget(m_addButton);
    
    m_clearButton = new QPushButton("Clear All", this);
    m_clearButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 10px; }");
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearAll);
    mainLayout->addWidget(m_clearButton);
    
    // === 性能统计 ===
    QGroupBox *statsGroup = new QGroupBox("Performance Stats", this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
    
    m_fpsLabel = new QLabel("FPS: 0", this);
    m_fpsLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #00FF00; }");
    statsLayout->addWidget(m_fpsLabel);
    
    m_trailCountLabel = new QLabel("Trails: 0", this);
    m_trailCountLabel->setStyleSheet("QLabel { font-size: 14px; }");
    statsLayout->addWidget(m_trailCountLabel);
    
    m_pointCountLabel = new QLabel("Total Points: 0", this);
    m_pointCountLabel->setStyleSheet("QLabel { font-size: 14px; }");
    statsLayout->addWidget(m_pointCountLabel);
    
    mainLayout->addWidget(statsGroup);
    
    // === 使用说明 ===
    QGroupBox *helpGroup = new QGroupBox("Controls", this);
    QVBoxLayout *helpLayout = new QVBoxLayout(helpGroup);
    
    QLabel *helpText = new QLabel(
        "• Left Mouse: Rotate view\n"
        "• Mouse Wheel: Zoom\n"
        "• Click 'Add Trail' to create\n"
        "• Adjust parameters before adding\n"
        "• Each trail has random color",
        this
    );
    helpText->setWordWrap(true);
    helpLayout->addWidget(helpText);
    
    mainLayout->addWidget(helpGroup);
    
    mainLayout->addStretch();
    
    splitter->addWidget(controlPanel);
    splitter->setStretchFactor(0, 3);  // 渲染窗口占3/4
    splitter->setStretchFactor(1, 1);  // 控制面板占1/4
    
    // 定时更新统计信息
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateStats);
    m_statsTimer->start(100);  // 100ms更新一次
}

MainWindow::~MainWindow()
{
}

void MainWindow::onAddTrail()
{
    int pointCount = m_pointCountSpinBox->value();
    float radius = m_radiusSpinBox->value();
    float width = m_widthSpinBox->value();
    
    m_trailWidget->addTrail(pointCount, radius, width);
}

void MainWindow::onClearAll()
{
    m_trailWidget->clearAllTrails();
}

void MainWindow::updateStats()
{
    m_fpsLabel->setText(QString("FPS: %1").arg(m_trailWidget->getCurrentFPS(), 0, 'f', 1));
    m_trailCountLabel->setText(QString("Trails: %1").arg(m_trailWidget->getTrailCount()));
    m_pointCountLabel->setText(QString("Total Points: %1").arg(m_trailWidget->getTotalPoints()));
}
