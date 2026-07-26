#pragma once

#include "Curve/Spline.h"
#include "Vectorization/Stages/Base/VectorizationStageBase.h"

#include <opencv2/core.hpp>

#include <QRandomGenerator>
#include <QVector2D>
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class ColorSampler : public VectorizationStageBase
    {
      public:
        explicit ColorSampler(QObject* pParent);

        void Run(const QVector<CurvePtr>& Curves, cv::Mat& Image, cv::Mat& ImageLab, const double SampleDensity);
        void Reset() override;

      private:
        void Sample(BezierPtr pBezier, cv::Mat& Image, cv::Mat& ImageLab, const double SampleDensity);
        void SampleAlongNormal(CurvePtr pCurve, float Parameter, ColorPointType Type, cv::Mat& Image, cv::Mat& ImageLab, const double Distance = 3.0);

      private:
        QRandomGenerator mRandomGenerator;
    };
}
