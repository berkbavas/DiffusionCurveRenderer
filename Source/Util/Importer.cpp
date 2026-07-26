#include "Importer.h"

#include "Curve/Spline.h"
#include "Util/Logger.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QVector<DiffusionCurveRenderer::CurvePtr> DiffusionCurveRenderer::Importer::ImportFromXml(const QString& Filename)
{
    QDomDocument Document;

    // Read the file
    QFile File(Filename);

    if (File.open(QIODevice::ReadOnly) == false)
    {
        qFatal() << "Importer::ImportFromXml: An error occured while loading '{}'" << Filename.toStdString();
        return {};
    }

    Document.setContent(&File);
    File.close();

    QDomElement Root = Document.documentElement();
    QDomElement Component = Root.firstChild().toElement();
    QVector<CurvePtr> Curves;

    while (Component.isNull() == false)
    {
        BezierPtr pCurve = std::make_shared<Bezier>();

        if (Component.tagName() == "curve")
        {
            QDomElement Child = Component.firstChild().toElement();
            while (Child.isNull() == false)
            {
                if (Child.tagName() == "control_points_set")
                {
                    QDomElement Element = Child.firstChild().toElement();
                    while (Element.isNull() == false)
                    {
                        const auto X = Element.attribute("y").toDouble();
                        const auto Y = Element.attribute("x").toDouble();
                        pCurve->AddControlPoint(QVector2D(X, Y));
                        Element = Element.nextSibling().toElement();
                    }
                }
                else if (Child.tagName() == "left_colors_set" || Child.tagName() == "right_colors_set")
                {
                    QDomElement Element = Child.firstChild().toElement();
                    int MaxGlobalId = 0;

                    QVector<QVector4D> Colors;
                    QVector<float> Positions;

                    while (Element.isNull() == false)
                    {
                        const auto R = Element.attribute("B").toUInt();
                        const auto G = Element.attribute("G").toUInt();
                        const auto B = Element.attribute("R").toUInt();
                        const int GlobalId = Element.attribute("globalID").toInt();

                        Colors << QVector4D(R / 255.0f, G / 255.0f, B / 255.0f, 1.0f);
                        Positions << GlobalId;

                        if (GlobalId >= MaxGlobalId)
                        {
                            MaxGlobalId = GlobalId;
                        }

                        Element = Element.nextSibling().toElement();
                    }

                    for (int I = 0; I < Colors.size(); ++I)
                    {
                        const auto Type = Child.tagName() == "left_colors_set" ? ColorPointType::Left : ColorPointType::Right;
                        pCurve->AddColorPoint(Type, Colors[I], Positions[I] / MaxGlobalId);
                    }
                }

                Child = Child.nextSibling().toElement();
            }
        }

        pCurve->AddBlurPoint(0, DEFAULT_BLUR_STRENGTH);
        pCurve->AddBlurPoint(0, DEFAULT_BLUR_STRENGTH);

        Curves << pCurve;

        Component = Component.nextSibling().toElement();
    }

    return Curves;
}

QVector<DiffusionCurveRenderer::CurvePtr> DiffusionCurveRenderer::Importer::ImportFromJson(const QString& Filename)
{
    // Read the file
    QFile File(Filename);

    if (File.open(QIODevice::ReadOnly) == false)
    {
        qFatal() << "Importer::ImportFromJson: An error occurred while loading '{}'" << Filename.toStdString();
        return {};
    }

    QJsonParseError ParseError;
    QJsonDocument Document = QJsonDocument::fromJson(File.readAll(), &ParseError);

    File.close();

    if (ParseError.error != QJsonParseError::NoError)
    {
        qFatal() << "Importer::ImportFromJson: Parse error at" << ParseError.offset << ":" << ParseError.errorString();
        return {};
    }

    const auto Root = Document.object();

    const auto Splines = Root.value("spline_curves").toArray();
    const auto Beziers = Root.value("bezier_curves").toArray();

    QVector<CurvePtr> Curves;
    for (const auto& Spline : Splines)
    {
        const auto SplineObject = Spline.toObject();

        if (SplineObject.isEmpty() == false)
        {
            Curves << Spline::FromJsonObject(SplineObject);
        }
    }

    for (const auto& Bezier : Beziers)
    {
        const auto BezierObject = Bezier.toObject();

        if (BezierObject.isEmpty() == false)
        {
            Curves << Bezier::FromJsonObject(BezierObject);
        }
    }

    return Curves;
}
