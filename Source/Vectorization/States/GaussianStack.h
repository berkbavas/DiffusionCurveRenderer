#pragma once

#include "Vectorization/States/VectorizationStateBase.h"

#include <QVector>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace DiffusionCurveRenderer
{
    class GaussianStack : public VectorizationStateBase
    {
        Q_OBJECT
      private:
        /*
         * A vector of increasingly Gaussian-blurred images.
         */
        QVector<cv::Mat> mLevels;

      public:
        explicit GaussianStack(QObject* parent);

        /**
         * Construct a Gaussian scale space representing the image passed in.
         *
         * Repeatedly applies a Gaussian filter with radius sigma starting from 0
         * and increasing by <sigmaStep> (default 0.4) each level. The stack stops
         * when the image's standard deviation drops below <stdDevCutoff> (default
         * 40) or, if a max height is specified, at that height.
         *
         * param image: An OpenCV matrix containing an RGB image.
         * param stdDevCutoff: The minimum standard deviation of a blurred image
         *                     that will be used in the stack.
         * param maxHeight: The maximum height of the stack.
         * param sigmaStep: The increase in Gaussian filter widths between each level.
         */
        void Run(cv::Mat image, double stdDevCutoff = 40.0, int maxHeight = 60, double sigmaStep = 0.4);

        /*
         *  Returns the number of levels in the scale space.
         */
        int GetHeight();

        /*
         *  Restricts the height of the stack to <layers> layers, removing any
         *  images above that level.
         */
        void Restrict(int layers);

        /*
         *  Returns the blurred RGB image at the <layer>'th layer.
         */
        cv::Mat GetLayer(int layer);

        void Reset() override;
    };
}