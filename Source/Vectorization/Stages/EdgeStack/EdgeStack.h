#pragma once

#include "Vectorization/Stages/GaussianStack/GaussianStack.h"

#include <opencv2/core/mat.hpp>

#include <QVector>

namespace DiffusionCurveRenderer
{
    class EdgeStack : public VectorizationStageBase
    {
      private:
        /*
         * A vector of edge images of increasingly Gaussian-blurred images.
         */
        std::vector<cv::Mat> mLevels;

      public:
        explicit EdgeStack(QObject* pParent);

        /**
         * Creates a stack of edge images from a Gaussian scale space.
         *
         * Runs Canny edge detection on each image in the Gaussian stack.
         * Edge detection uses low and high thresholds as specified by
         * parameters.
         *
         * param pStack: A stack of images from a Gaussian scale space.
         * param LowThreshold: Low edge strength threshold for Canny edges.
         * param HighThreshold: High edge strength threshold for Canny edges.
         */
        void Run(GaussianStack* pStack, double LowThreshold, double HighThreshold);

        /*
         *  Returns the number of levels in the edge stack.
         */
        int GetHeight();

        /*
         *  Returns the image of edges at the <layer>'th layer.
         */
        cv::Mat GetLayer(int Layer);

        void Reset() override;
    };
}
