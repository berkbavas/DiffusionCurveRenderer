#pragma once

#include "Bezier.h"
#include <opencv2/core.hpp>

#include <QRandomGenerator>
#include <QVector2D>
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class ColorSampler
    {
    public:
        ColorSampler();
        void Run(const QVector<Bezier*>& curves, cv::Mat& image, cv::Mat& imageLab, const double sampleDensity);

    private:
        void SampleAlongNormal(Bezier* curve, float parameter, ColorPoint::Direction direction, cv::Mat& image, cv::Mat& imageLab, const double distance = 3.0);

    private:
        float mProgress;
        QRandomGenerator mRandomGenerator;
    };
}
