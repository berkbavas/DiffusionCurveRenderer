#pragma once

#include "Bezier.h"

#include <QByteArray>

namespace DiffusionCurveRenderer
{
    class Helper
    {
        Helper();

    public:
        static QByteArray GetBytes(QString path);
        static QVector<Bezier*> LoadCurveDataFromXML(const QString& filename);
        static QVector<Bezier*> LoadCurveDataFromJSON(const QString& filename);
        static bool SaveCurveDataToJSON(const QVector<Bezier*>& curves, const QString& filename);
    };
}