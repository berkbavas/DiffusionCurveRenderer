#include "Helper.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

DiffusionCurveRenderer::Helper::Helper() {}

QByteArray DiffusionCurveRenderer::Helper::GetBytes(QString path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
        return file.readAll();
    }
    else
    {
        qWarning() << QString("Could not open '%1'").arg(path);
        return QByteArray();
    }
}

QVector<DiffusionCurveRenderer::Bezier*> DiffusionCurveRenderer::Helper::LoadCurveDataFromXML(const QString& filename)
{
    QDomDocument document;

    // Read the file
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qCritical() << "Error occured while loading the file:" << filename;
            return QVector<Bezier*>();
        }

        document.setContent(&file);
        file.close();
    }

    QDomElement root = document.documentElement();
    QDomElement component = root.firstChild().toElement();
    QVector<Bezier*> curves;
    while (!component.isNull())
    {
        Bezier* curve = new Bezier;
        if (component.tagName() == "curve")
        {
            QDomElement child = component.firstChild().toElement();
            while (!child.isNull())
            {
                if (child.tagName() == "control_points_set")
                {
                    QDomElement element = child.firstChild().toElement();
                    while (!element.isNull())
                    {
                        float x = element.attribute("y").toDouble();
                        float y = element.attribute("x").toDouble();
                        ControlPoint* controlPoint = new ControlPoint;
                        controlPoint->mPosition = QVector2D(x, y);
                        curve->AddControlPoint(controlPoint);
                        element = element.nextSibling().toElement();
                    }
                }
                else if (child.tagName() == "left_colors_set" || child.tagName() == "right_colors_set")
                {
                    QDomElement element = child.firstChild().toElement();
                    int maxGlobalID = 0;

                    QVector<ColorPoint*> colorPoints;

                    while (!element.isNull())
                    {
                        uint8_t r = element.attribute("B").toUInt();
                        uint8_t g = element.attribute("G").toUInt();
                        uint8_t b = element.attribute("R").toUInt();
                        int globalID = element.attribute("globalID").toInt();

                        ColorPoint* colorPoint = new ColorPoint;
                        colorPoint->SetParent(curve);
                        colorPoint->mColor = QVector4D(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
                        colorPoint->mDirection = child.tagName() == "left_colors_set" ? ColorPoint::Direction::Left : ColorPoint::Direction::Right;
                        colorPoint->mPosition = globalID;
                        colorPoints << colorPoint;

                        if (globalID >= maxGlobalID)
                            maxGlobalID = globalID;

                        element = element.nextSibling().toElement();
                    }

                    for (int i = 0; i < colorPoints.size(); ++i)
                    {
                        colorPoints[i]->mPosition = colorPoints[i]->mPosition / maxGlobalID;
                        curve->AddColorPoint(colorPoints[i]);
                    }
                }
                child = child.nextSibling().toElement();
            }
        }

        BlurPoint* blurPoint0 = new BlurPoint;
        blurPoint0->SetParent(curve);
        blurPoint0->mPosition = 0.0f;
        curve->AddBlurPoint(blurPoint0);

        BlurPoint* blurPoint1 = new BlurPoint;
        blurPoint1->SetParent(curve);
        blurPoint1->mPosition = 1.0f;
        curve->AddBlurPoint(blurPoint1);

        curves << curve;
        component = component.nextSibling().toElement();
    }

    return curves;
}

QVector<DiffusionCurveRenderer::Bezier*> DiffusionCurveRenderer::Helper::LoadCurveDataFromJSON(const QString& filename)
{
    QJsonDocument document;

    // Read the file
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qCritical() << "Error occured while loading the file:" << filename;
            return QVector<Bezier*>();
        }

        document = QJsonDocument::fromJson(file.readAll());
        file.close();
    }

    QVector<Bezier*> curves;
    QJsonArray curvesArray = document.array();

    for (auto element : curvesArray)
    {
        Bezier* curve = new Bezier;

        QJsonObject curveObject = element.toObject();
        QJsonArray leftColorsArray = curveObject["left_color_points"].toArray();
        QJsonArray rightColorsArray = curveObject["right_color_points"].toArray();
        QJsonArray controlPointsArray = curveObject["control_points"].toArray();
        QJsonArray blurPointsArray = curveObject["blur_points"].toArray();
        int z = curveObject["z"].toInt();

        // Control points
        for (auto controlPointElement : controlPointsArray)
        {
            QJsonObject controlPointObject = controlPointElement.toObject();
            QJsonObject positionObject = controlPointObject["position"].toObject();
            float x = positionObject["x"].toDouble();
            float y = positionObject["y"].toDouble();

            ControlPoint* controlPoint = new ControlPoint;
            controlPoint->mPosition = QVector2D(x, y);
            curve->AddControlPoint(controlPoint);
        }

        // Blur points
        for (auto blurPointElement : blurPointsArray)
        {
            QJsonObject blurPointObject = blurPointElement.toObject();
            float position = blurPointObject["position"].toDouble();
            float strength = blurPointObject["strength"].toDouble();

            BlurPoint* point = new BlurPoint;
            point->mPosition = position;
            point->mStrength = strength;
            point->SetParent(curve);
            curve->AddBlurPoint(point);
        }

        // Left colors
        for (auto leftColorElement : leftColorsArray)
        {
            QJsonObject leftColorObject = leftColorElement.toObject();
            QJsonObject colorObject = leftColorObject["color"].toObject();
            float r = colorObject["r"].toDouble();
            float g = colorObject["g"].toDouble();
            float b = colorObject["b"].toDouble();
            float a = colorObject["a"].toDouble();
            float position = leftColorObject["position"].toDouble();

            ColorPoint* colorPoint = new ColorPoint;
            colorPoint->mDirection = ColorPoint::Direction::Left;
            colorPoint->mColor = QVector4D(r, g, b, a);
            colorPoint->mPosition = position;
            colorPoint->SetParent(curve);
            curve->AddColorPoint(colorPoint);
        }

        // Right colors
        for (auto rightColorElement : rightColorsArray)
        {
            QJsonObject rightColorObject = rightColorElement.toObject();
            QJsonObject colorObject = rightColorObject["color"].toObject();
            float r = colorObject["r"].toDouble();
            float g = colorObject["g"].toDouble();
            float b = colorObject["b"].toDouble();
            float a = colorObject["a"].toDouble();
            float position = rightColorObject["position"].toDouble();

            ColorPoint* colorPoint = new ColorPoint;
            colorPoint->mDirection = ColorPoint::Direction::Right;
            colorPoint->mColor = QVector4D(r, g, b, a);
            colorPoint->mPosition = position;
            colorPoint->SetParent(curve);
            curve->AddColorPoint(colorPoint);
        }

        curve->mDepth = z;
        curves << curve;
    }

    return curves;
}

bool DiffusionCurveRenderer::Helper::SaveCurveDataToJSON(const QVector<Bezier*>& curves, const QString& filename)
{
    QJsonArray curvesArray;

    for (const auto curve : curves)
    {
        QJsonObject curveObject;
        QJsonArray controlPointsArray;
        QJsonArray leftColorsArray;
        QJsonArray rightColorsArray;
        QJsonArray blurPointsArray;

        // Control points
        QVector<ControlPoint*> controlPoints = curve->GetControlPoints();

        for (const auto controlPoint : controlPoints)
        {
            QJsonObject controlPointObject;

            QJsonObject position;
            position.insert("x", controlPoint->mPosition.x());
            position.insert("y", controlPoint->mPosition.y());

            controlPointObject.insert("position", position);

            controlPointsArray << controlPointObject;
        }

        // Blur points
        QVector<BlurPoint*> blurPoints = curve->GetBlurPoints();

        for (const auto blurPoint : blurPoints)
        {
            QJsonObject blurPointObject;

            blurPointObject.insert("position", blurPoint->mPosition);
            blurPointObject.insert("strength", blurPoint->mStrength);

            blurPointsArray << blurPointObject;
        }

        // Left colors
        QList<ColorPoint*> leftColors = curve->GetLeftColorPoints();

        for (const auto leftColor : leftColors)
        {
            QJsonObject leftColorObject;

            QJsonObject color;
            color.insert("r", leftColor->mColor.x());
            color.insert("g", leftColor->mColor.y());
            color.insert("b", leftColor->mColor.z());
            color.insert("a", leftColor->mColor.w());

            leftColorObject.insert("color", color);
            leftColorObject.insert("position", leftColor->mPosition);

            leftColorsArray << leftColorObject;
        }

        // Right colors
        QList<ColorPoint*> rightColors = curve->GetRightColorPoints();

        for (const auto rightColor : rightColors)
        {
            QJsonObject rightColorObject;

            QJsonObject color;
            color.insert("r", rightColor->mColor.x());
            color.insert("g", rightColor->mColor.y());
            color.insert("b", rightColor->mColor.z());
            color.insert("a", rightColor->mColor.w());

            rightColorObject.insert("color", color);
            rightColorObject.insert("position", rightColor->mPosition);

            rightColorsArray << rightColorObject;
        }

        curveObject.insert("z", curve->mDepth);
        curveObject.insert("control_points", controlPointsArray);
        curveObject.insert("left_color_points", leftColorsArray);
        curveObject.insert("right_color_points", rightColorsArray);
        curveObject.insert("blur_points", blurPointsArray);
        curvesArray << curveObject;
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonDocument document;
        document.setArray(curvesArray);
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << document.toJson(QJsonDocument::Indented);
        stream.flush();
        file.close();
        return true;
    }
    else
    {
        qCritical() << Q_FUNC_INFO << "Couldn't write to file" << filename;
        return false;
    }
}