#include <QApplication>
#include <QSurfaceFormat>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置OpenGL 4.3 Core Profile
    QSurfaceFormat format;
    format.setVersion(4, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
