#pragma once

#include <QByteArray>
#include <QString>

namespace DiffusionCurveRenderer
{
    class Util
    {
      public:
        Util() = delete;

        static QByteArray GetBytes(const QString& path);
    };
}
