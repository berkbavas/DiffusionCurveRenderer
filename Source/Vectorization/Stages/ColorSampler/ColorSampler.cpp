#include "ColorSampler.h"

DiffusionCurveRenderer::ColorSampler::ColorSampler(QObject* parent)
    : VectorizationStageBase(parent)
{
    mRandomGenerator = QRandomGenerator::securelySeeded();
}

void DiffusionCurveRenderer::ColorSampler::Run(const QVector<CurvePtr>& curves, cv::Mat& image, cv::Mat& imageLab, const double sampleDensity)
{
    int nCurves = curves.size();

    for (int i = 0; i < nCurves; i++)
    {

        CurvePtr curve = curves[i];

        if (BezierPtr bezier = std::dynamic_pointer_cast<Bezier>(curve))
        {
            Sample(bezier, image, imageLab, sampleDensity);
        }
        else if (SplinePtr spline = std::dynamic_pointer_cast<Spline>(curve))
        {
            for (const auto& bezier : spline->GetBezierPatches())
            {
                Sample(bezier, image, imageLab, sampleDensity);
            }
        }

        emit ProgressChanged(float(i) / (nCurves - 1));
    }
}

void DiffusionCurveRenderer::ColorSampler::Sample(BezierPtr bezier, cv::Mat& image, cv::Mat& imageLab, const double sampleDensity)
{
    SampleAlongNormal(bezier, 0.0f, ColorPointType::Left, image, imageLab);
    SampleAlongNormal(bezier, 0.0f, ColorPointType::Right, image, imageLab);

    SampleAlongNormal(bezier, 0.5, ColorPointType::Left, image, imageLab);
    SampleAlongNormal(bezier, 0.5, ColorPointType::Right, image, imageLab);

    SampleAlongNormal(bezier, 1.0f, ColorPointType::Left, image, imageLab);
    SampleAlongNormal(bezier, 1.0f, ColorPointType::Right, image, imageLab);

    int nSamples = sampleDensity * bezier->CalculateLength();

    for (int i = 0; i < nSamples - 3; i++)
    {
        SampleAlongNormal(bezier, mRandomGenerator.bounded(1.0f), ColorPointType::Left, image, imageLab);
        SampleAlongNormal(bezier, mRandomGenerator.bounded(1.0f), ColorPointType::Right, image, imageLab);
    }
}

void DiffusionCurveRenderer::ColorSampler::SampleAlongNormal(CurvePtr curve, float parameter, ColorPointType type, cv::Mat& image, cv::Mat& imageLab, const double distance)
{
    const int width = image.cols;
    const int height = image.rows;
    QVector2D point = curve->PositionAt(parameter);
    QVector2D normal = curve->NormalAt(parameter);

    if (type == ColorPointType::Right)
        normal = -normal;

    // Traverse the (normalized) normal to a sample point.
    QVector2D sample = (point + distance * normal);
    sample[0] = int(sample[0]);
    sample[1] = int(sample[1]);

    // Check that the sample point is inside the image.
    bool xOutOfBounds = sample.x() < 0 || sample.x() >= width;
    bool yOutOfBounds = sample.y() < 0 || sample.y() >= height;
    if (xOutOfBounds || yOutOfBounds)
    {
        // Return null to signify that no colour could be sampled here.
        return;
    }

    auto color = &image.at<cv::Vec3b>(sample.y(), sample.x());

    if (color != nullptr)
    {
        // Colour sample is valid.;
        const auto color4d = QVector4D(int(color->val[2]) / 255.0f, int(color->val[1]) / 255.0f, int(color->val[0]) / 255.0f, 1.0);
        curve->AddColorPoint(type, color4d, parameter);
    }
}

void DiffusionCurveRenderer::ColorSampler::Reset()
{
    // Nothing to do
}
