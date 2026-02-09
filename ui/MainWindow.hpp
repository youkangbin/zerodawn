#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include "TrailWidget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTrail();
    void onClearAll();
    void updateStats();

private:
    TrailWidget *m_trailWidget;
    
    // UI控件
    QSpinBox *m_pointCountSpinBox;
    QDoubleSpinBox *m_radiusSpinBox;
    QDoubleSpinBox *m_widthSpinBox;
    QPushButton *m_addButton;
    QPushButton *m_clearButton;
    
    QLabel *m_fpsLabel;
    QLabel *m_trailCountLabel;
    QLabel *m_pointCountLabel;
    
    QTimer *m_statsTimer;
};

#endif // MAINWINDOW_H
