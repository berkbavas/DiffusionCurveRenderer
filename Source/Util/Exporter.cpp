#include "Exporter.h"

#include "Curve/Spline.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

bool DiffusionCurveRenderer::Exporter::ExportAsJson(QVector<CurvePtr> curves, const QString& filename)
{
    QJsonArray splines;
    QJsonArray beziers;

    for (const auto& curve : curves)
    {
        if (SplinePtr spline = std::dynamic_pointer_cast<Spline>(curve))
        {
            splines.append(spline->ToJsonObject());
        }
        else if (BezierPtr bezier = std::dynamic_pointer_cast<Bezier>(curve))
        {
            beziers.append(bezier->ToJsonObject());
        }
    }

    QJsonObject root;

    root.insert("spline_curves", splines);
    root.insert("bezier_curves", beziers);

    QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);

    QFile file;
    file.setFileName(filename);

    if (file.open(QIODevice::WriteOnly) == false)
    {
        qFatal() << "Exporter::Export: No write access to file '{}'" << filename.toStdString();
        return false;
    }

    file.write(bytes);
    file.close();

    return true;
}