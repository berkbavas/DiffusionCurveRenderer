#pragma once

#include "Curve/Curve.h"

namespace DiffusionCurveRenderer
{
    class Exporter
    {
      public:
        Exporter() = delete;

        static bool ExportAsJson(QVector<CurvePtr> curves, const QString& filename);
    };
}