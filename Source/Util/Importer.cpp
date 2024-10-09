#include "Importer.h"

#include "Curve/Spline.h"
#include "Util/Logger.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QVector<DiffusionCurveRenderer::CurvePtr> DiffusionCurveRenderer::Importer::ImportFromXml(const QString& filename)
{
    QDomDocument document;

    // Read the file
    QFile file(filename);

    if (file.open(QIODevice::ReadOnly) == false)
    {
        LOG_FATAL("An error occured while loading '{}'", filename.toStdString());
        return {};
    }

    document.setContent(&file);
    file.close();

    QDomElement root = document.documentElement();
    QDomElement component = root.firstChild().toElement();
    QVector<CurvePtr> curves;

    while (component.isNull() == false)
    {
        BezierPtr curve = std::make_shared<Bezier>();

        if (component.tagName() == "curve")
        {
            QDomElement child = component.firstChild().toElement();
            while (child.isNull() == false)
            {
                if (child.tagName() == "control_points_set")
                {
                    QDomElement element = child.firstChild().toElement();
                    while (element.isNull() == false)
                    {
                        const float x = element.attribute("y").toDouble();
                        const float y = element.attribute("x").toDouble();

                        curve->AddControlPoint(QVector2D(x, y));

                        element = element.nextSibling().toElement();
                    }
                }
                else if (child.tagName() == "left_colors_set" || child.tagName() == "right_colors_set")
                {
                    QDomElement element = child.firstChild().toElement();
                    int maxGlobalID = 0;

                    QVector<QVector4D> colors;
                    QVector<float> positions;

                    while (element.isNull() == false)
                    {
                        const uint8_t r = element.attribute("B").toUInt();
                        const uint8_t g = element.attribute("G").toUInt();
                        const uint8_t b = element.attribute("R").toUInt();
                        const int globalID = element.attribute("globalID").toInt();

                        colors << QVector4D(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
                        positions << globalID;

                        if (globalID >= maxGlobalID)
                        {
                            maxGlobalID = globalID;
                        }

                        element = element.nextSibling().toElement();
                    }

                    for (int i = 0; i < colors.size(); ++i)
                    {
                        const auto type = child.tagName() == "left_colors_set" ? ColorPointType::Left : ColorPointType::Right;
                        curve->AddColorPoint(type, colors[i], positions[i] / maxGlobalID);
                    }
                }
                child = child.nextSibling().toElement();
            }
        }

        curve->AddBlurPoint(0, DEFAULT_BLUR_STRENGTH);
        curve->AddBlurPoint(0, DEFAULT_BLUR_STRENGTH);

        curves << curve;

        component = component.nextSibling().toElement();
    }

    return curves;
}