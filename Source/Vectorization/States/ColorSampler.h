#pragma once

#include "Curve/Spline.h"

#include <QRandomGenerator>
#include <QVector2D>
#include <QVector4D>
#include <opencv2/core.hpp>

namespace DiffusionCurveRenderer
{
    class ColorSampler
    {
      public:
        ColorSampler();
        void Run(const QVector<CurvePtr>& curves, cv::Mat& image, cv::Mat& imageLab, const double sampleDensity);

      private:
        void SampleAlongNormal(CurvePtr curve, float parameter, ColorPointType type, cv::Mat& image, cv::Mat& imageLab, const double distance = 3.0);

      private:
        QRandomGenerator mRandomGenerator;
    };
}
