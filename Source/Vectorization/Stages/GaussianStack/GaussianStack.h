#pragma once

#include "Vectorization/Stages/Base/VectorizationStageBase.h"

#include <QVector>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace DiffusionCurveRenderer
{
    class GaussianStack : public VectorizationStageBase
    {
        Q_OBJECT
      private:
        /*
         * A vector of increasingly Gaussian-blurred images.
         */
        QVector<cv::Mat> mLevels;

      public:
        explicit GaussianStack(QObject* pParent);

        /**
         * Construct a Gaussian scale space representing the image passed in.
         *
         * Repeatedly applies a Gaussian filter with radius sigma starting from 0
         * and increasing by <sigmaStep> (default 0.4) each level. The stack stops
         * when the image's standard deviation drops below <stdDevCutoff> (default
         * 40) or, if a max height is specified, at that height.
         *
         * param Image: An OpenCV matrix containing an RGB image.
         * param StdDevCutoff: The minimum standard deviation of a blurred image
         *                     that will be used in the stack.
         * param MaxHeight: The maximum height of the stack.
         * param SigmaStep: The increase in Gaussian filter widths between each level.
         */
        void Run(cv::Mat Image, double StdDevCutoff = 40.0, int MaxHeight = 60, double SigmaStep = 0.4);

        /*
         *  Returns the number of levels in the scale space.
         */
        int GetHeight();

        /*
         *  Restricts the height of the stack to <layers> layers, removing any
         *  images above that level.
         */
        void Restrict(int Layers);

        /*
         *  Returns the blurred RGB image at the <layer>'th layer.
         */
        cv::Mat GetLayer(int Layer);

        void Reset() override;
    };
}