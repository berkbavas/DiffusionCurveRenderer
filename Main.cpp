#include "Controller.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <QScreen>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    Controller *controller = new Controller;
    controller->init();

    return app.exec();
}
