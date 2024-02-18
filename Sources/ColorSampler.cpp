#include "ColorSampler.h"

#include <random>

DiffusionCurveRenderer::ColorSampler::ColorSampler()
    : mProgress(0.0f)
{
    mRandomGenerator = QRandomGenerator::securelySeeded();
}

void DiffusionCurveRenderer::ColorSampler::Run(const QVector<Bezier*>& curves, cv::Mat& image, cv::Mat& imageLab, const double sampleDensity)
{
    for (int i = 0; i < curves.size(); i++)
    {
        mProgress = float(i) / (curves.size() - 1);

        Bezier* curve = curves[i];

        SampleAlongNormal(curve, 0.0f, ColorPoint::Direction::Left, image, imageLab);
        SampleAlongNormal(curve, 0.0f, ColorPoint::Direction::Right, image, imageLab);

        SampleAlongNormal(curve, 0.5, ColorPoint::Direction::Left, image, imageLab);
        SampleAlongNormal(curve, 0.5, ColorPoint::Direction::Right, image, imageLab);

        SampleAlongNormal(curve, 1.0f, ColorPoint::Direction::Left, image, imageLab);
        SampleAlongNormal(curve, 1.0f, ColorPoint::Direction::Right, image, imageLab);

        int nSamples = sampleDensity * curve->GetLength();

        for (int i = 0; i < nSamples - 3; i++)
        {
            SampleAlongNormal(curve, mRandomGenerator.bounded(1.0f), ColorPoint::Direction::Left, image, imageLab);
            SampleAlongNormal(curve, mRandomGenerator.bounded(1.0f), ColorPoint::Direction::Right, image, imageLab);
        }
    }
}

void DiffusionCurveRenderer::ColorSampler::SampleAlongNormal(Bezier* curve, float parameter, ColorPoint::Direction direction, cv::Mat& image, cv::Mat& imageLab, const double distance)
{
    const int width = image.cols;
    const int height = image.rows;
    QVector2D point = curve->ValueAt(parameter);
    QVector2D normal = curve->NormalAt(parameter);

    if (direction == ColorPoint::Direction::Right)
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
        // Colour sample is valid.
        ColorPoint* point = new ColorPoint;
        point->mColor = QVector4D(int(color->val[2]) / 255.0f, int(color->val[1]) / 255.0f, int(color->val[0]) / 255.0f, 1.0);
        point->mDirection = direction;
        point->mPosition = parameter;
        point->SetParent(curve);
        curve->AddColorPoint(point);
    }
}