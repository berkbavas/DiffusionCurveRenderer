#include "Window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    DiffusionCurveRenderer::Window w;
    w.resize(1600, 900);
    w.showMaximized();

    return a.exec();
}
