#include <QApplication>
#include <QSurfaceFormat>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set default OpenGL format
    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setAlphaBufferSize(8); // For transparency
    format.setSwapInterval(0); 
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow window;
    window.show();
    
    return app.exec();
}
