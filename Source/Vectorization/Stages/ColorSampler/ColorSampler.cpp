#include "ColorSampler.h"

DiffusionCurveRenderer::ColorSampler::ColorSampler(QObject* parent)
    : VectorizationStageBase(parent)
{
    mRandomGenerator = QRandomGenerator::securelySeeded();
}

void DiffusionCurveRenderer::ColorSampler::Run(const QVector<CurvePtr>& Curves, cv::Mat& Image, cv::Mat& ImageLab, const double SampleDensity)
{
    int nCurves = Curves.size();

    for (int I = 0; I < nCurves; I++)
    {
        CurvePtr pCurve = Curves[I];

        if (BezierPtr pBezier = std::dynamic_pointer_cast<Bezier>(pCurve))
        {
            Sample(pBezier, Image, ImageLab, SampleDensity);
        }
        else if (SplinePtr pSpline = std::dynamic_pointer_cast<Spline>(pCurve))
        {
            for (const auto& pBezier : pSpline->GetBezierPatches())
            {
                Sample(pBezier, Image, ImageLab, SampleDensity);
            }
        }

        emit ProgressChanged(float(I) / (nCurves - 1));
    }
}

void DiffusionCurveRenderer::ColorSampler::Sample(BezierPtr pBezier, cv::Mat& Image, cv::Mat& ImageLab, const double SampleDensity)
{
    SampleAlongNormal(pBezier, 0.0f, ColorPointType::Left, Image, ImageLab);
    SampleAlongNormal(pBezier, 0.0f, ColorPointType::Right, Image, ImageLab);

    SampleAlongNormal(pBezier, 0.5, ColorPointType::Left, Image, ImageLab);
    SampleAlongNormal(pBezier, 0.5, ColorPointType::Right, Image, ImageLab);

    SampleAlongNormal(pBezier, 1.0f, ColorPointType::Left, Image, ImageLab);
    SampleAlongNormal(pBezier, 1.0f, ColorPointType::Right, Image, ImageLab);

    int nSamples = SampleDensity * pBezier->CalculateLength();

    for (int i = 0; i < nSamples - 3; i++)
    {
        SampleAlongNormal(pBezier, mRandomGenerator.bounded(1.0f), ColorPointType::Left, Image, ImageLab);
        SampleAlongNormal(pBezier, mRandomGenerator.bounded(1.0f), ColorPointType::Right, Image, ImageLab);
    }
}

void DiffusionCurveRenderer::ColorSampler::SampleAlongNormal(CurvePtr pCurve, float Parameter, ColorPointType Type, cv::Mat& Image, cv::Mat& ImageLab, const double Distance)
{
    const auto Width = Image.cols;
    const auto Height = Image.rows;
    QVector2D Point = pCurve->PositionAt(Parameter);
    QVector2D Normal = pCurve->NormalAt(Parameter);

    if (Type == ColorPointType::Right)
    {
        Normal = -Normal;
    }

    // Traverse the (normalized) normal to a sample point.
    QVector2D Sample = (Point + Distance * Normal);
    Sample[0] = int(Sample[0]);
    Sample[1] = int(Sample[1]);

    // Check that the sample point is inside the image.
    bool xOutOfBounds = Sample.x() < 0 || Sample.x() >= Width;
    bool yOutOfBounds = Sample.y() < 0 || Sample.y() >= Height;
    if (xOutOfBounds || yOutOfBounds)
    {
        // Return null to signify that no colour could be sampled here.
        return;
    }

    auto Color = &Image.at<cv::Vec3b>(Sample.y(), Sample.x());

    if (Color != nullptr)
    {
        // Colour sample is valid.;
        const auto Color4d = QVector4D(int(Color->val[2]) / 255.0f, int(Color->val[1]) / 255.0f, int(Color->val[0]) / 255.0f, 1.0);
        pCurve->AddColorPoint(Type, Color4d, Parameter);
    }
}

void DiffusionCurveRenderer::ColorSampler::Reset()
{
    // Nothing to do
}
