#include "Util.h"

#include "Util/Logger.h"

#include <QFile>

QByteArray DiffusionCurveRenderer::Util::GetBytes(const QString& Path)
{
    QFile File(Path);

    if (File.open(QFile::ReadOnly))
    {
        return File.readAll();
    }
    else
    {
        DCR_EXIT_FAILURE("Failed to open file: {}", Path.toStdString());
    }
}
