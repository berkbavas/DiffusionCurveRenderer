#include "Exporter.h"

#include "Curve/Spline.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

bool DiffusionCurveRenderer::Exporter::ExportAsJson(const QVector<CurvePtr>& Curves, const QString& Filename)
{
    QJsonArray Splines;
    QJsonArray Beziers;

    for (const auto& pCurve : Curves)
    {
        if (SplinePtr pSpline = std::dynamic_pointer_cast<Spline>(pCurve))
        {
            Splines.append(pSpline->ToJsonObject());
        }
        else if (BezierPtr pBezier = std::dynamic_pointer_cast<Bezier>(pCurve))
        {
            Beziers.append(pBezier->ToJsonObject());
        }
    }

    QJsonObject Root;

    Root.insert("spline_curves", Splines);
    Root.insert("bezier_curves", Beziers);

    QByteArray Bytes = QJsonDocument(Root).toJson(QJsonDocument::Indented);

    QFile File;
    File.setFileName(Filename);

    if (File.open(QIODevice::WriteOnly) == false)
    {
        qFatal() << "Exporter::ExportAsJson: No write access to file '{}'" << Filename.toStdString();
        return false;
    }

    File.write(Bytes);
    File.close();

    return true;
}
