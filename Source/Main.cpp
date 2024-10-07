#include "Core/Controller.h"
#include "Util/Logger.h"

#include <QApplication>
#include <QImageReader>

using namespace DiffusionCurveRenderer;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    qInstallMessageHandler(Logger::QtMessageOutputCallback);

    QImageReader::setAllocationLimit(0);

    Controller controller;

    controller.Run();

    return app.exec();
}
