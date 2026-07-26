#pragma once

#include "Curve/Curve.h"

namespace DiffusionCurveRenderer
{
    class Exporter
    {
      public:
        Exporter() = delete;

        static bool ExportAsJson(const QVector<CurvePtr>& Curves, const QString& Filename);
    };
}