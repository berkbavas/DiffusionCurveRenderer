#include "Core/Controller.h"

#include <QApplication>

int main(int Argc, char* pArgv[])
{
    QApplication Application(Argc, pArgv);
    DiffusionCurveRenderer::Controller Controller;
    Controller.Run();

    return Application.exec();
}
