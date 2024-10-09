#pragma once

#include "Curve/Curve.h"

namespace DiffusionCurveRenderer
{
    class Importer
    {
      public:
        Importer() = delete;

        static QVector<CurvePtr> ImportFromXml(const QString& filename);
    };
}