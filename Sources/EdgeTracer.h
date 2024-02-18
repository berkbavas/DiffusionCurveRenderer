#pragma once

#include "PixelChain.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include <QList>

namespace DiffusionCurveRenderer
{
    class EdgeTracer
    {
    public:
        EdgeTracer();

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

        float GetProgress() const;

        const QList<PixelChain>& GetChains() const;

    private:
        float mProgress; // [0,..., 1]
        QList<PixelChain> mChains;
    };
}
