#ifndef UTIL_H
#define UTIL_H

#include <Curves/Curve.h>
#
class Util
{
public:
    Util();

    static QVector<Curve *> loadCurveDataFromXml(const QString &filename);
    static QVector<Curve *> loadCurveDataFromJson(const QString &filename);
    static bool saveCurveDataToJson(const QVector<Curve *> &curves, const QString &filename);
};

#endif // UTIL_H
