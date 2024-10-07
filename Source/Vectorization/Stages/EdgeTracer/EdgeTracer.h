#pragma once

#include "Vectorization/Stages/Base/PixelChain.h"
#include "Vectorization/Stages/Base/VectorizationStageBase.h"

#include <QList>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

namespace DiffusionCurveRenderer
{
    class EdgeTracer : public VectorizationStageBase
    {
      public:
        explicit EdgeTracer(QObject* parent);

        /*
         * This code taken from https://github.com/zhuethanca/DiffusionCurves
         *
         * Returns a series of distinct chains of pixels from the edge images. Each
         * chain represents a whole edge, parametrized into a 1D representation.
         *
         * Edges of length less than <lengthThreshold> are discarded.
         *
         * param chains: Output vector of edge pixel chains.
         * param edges: Black-and-white image, where edges are identified by white pixels.
         * param lengthThreshold: Minimum length required for an edge to be returned.
         */
        void Run(cv::Mat edges, int lengthThreshold);

        const QList<PixelChain>& GetChains() const;

        void Reset() override;

      private:
        QList<PixelChain> mChains;
    };
}
