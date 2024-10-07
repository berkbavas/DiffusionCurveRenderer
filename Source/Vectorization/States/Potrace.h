#pragma once

#include "PixelChain.h"
#include "Vectorization/States/VectorizationStateBase.h"

#include <QVector>

namespace DiffusionCurveRenderer
{
    class Potrace : public VectorizationStateBase
    {
      public:
        explicit Potrace(QObject* parent);

        /*
         * This code taken from https://github.com/zhuethanca/DiffusionCurves
         *
         * Runs a modified Potrace (polygon trace) algorithm to transform a continuous
         * pixel chain into a polyline with minimal segments while still respecting the
         * shape of the pixel chain.
         *
         * For each pixel, the algorithm attempts to construct a straight line to every
         * other pixel. Each attempt has a penalty based on the standard deviation of
         * pixels between the start and end from the line segment. Lines with too high
         * a penalty are discarded. Short polylines are prioritized over low error
         * among valid paths.
         *
         * param polyline: Return value which will hold the points in the polyline.
         * param chain: Continuous chain of pixels to be approximated by a polyline.
         */
        void Run(const QVector<PixelChain>& chains);

        const QVector<QVector<Point>>& GetPolylines() const;

        void Reset() override;

      private:
        /* This code taken from https://github.com/zhuethanca/DiffusionCurves
         *
         * Finds a sequence of pixels, forming a polyline, which approximates
         * the pixel chain <chain> with the minimum number of segments and with
         * the minimum penalty given the matrix <penalties>.
         *
         * param bestPath: Return value, a sequence of points approximating the chain.
         * param chain: Continuous chain of pixels to be approximated by a polyline.
         * param penalties: Matrix in which the entry i, j represents the penalty of
         *                  a straight line between pixels i and j, or -1 if they are
         *                  not connectable by a straight line.
         */
        void FindBestPath(QVector<Point>& bestPath, PixelChain chain, Eigen::MatrixXd penalties);

      private:
        QVector<QVector<Point>> mPolylines;
    };
}
