#include "Potrace.h"

DiffusionCurveRenderer::Potrace::Potrace(QObject* parent)
    : VectorizationStateBase(parent)
{
}

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
void DiffusionCurveRenderer::Potrace::Run(const QVector<PixelChain>& chains)
{
    for (int i = 0; i < chains.size(); i++)
    {
        float progress = float(i) / (chains.size() - 1);

        emit ProgressChanged(progress);

        PixelChain chain = chains[i];
        QVector<Point> polyline;

        const int nPoints = chain.GetLength();

        // Each entry i, j represents the penalty of a straight line
        // from pixel i directly to pixel j.
        Eigen::MatrixXd penalties(nPoints, nPoints);
        // Set a recognizable dummy value to signify no penalty is set.
        penalties.setConstant(-1.0);

        // First, find a penalty between each pair of pixels, or leave the dummy
        // value if two pixels cannot be connected by a straight line.

        // Iterate over all triplets i, j, k, such that 0 <= i < j < k <= nPoints.
        for (int i = 0; i < nPoints; i++)
        {
            for (int k = i + 1; k < nPoints; k++)
            {
                bool isStraightPath = true;

                Eigen::Vector2f pointI = chain.Get(i).ToVector();
                Eigen::Vector2f pointK = chain.Get(k).ToVector();

                // Check that all points between I and K are close enough to the
                // straight line connecting them.
                for (int j = i + 1; j < k; j++)
                {
                    Eigen::Vector2f pointJ = chain.Get(j).ToVector();

                    Eigen::Vector2f lineItoK = pointK - pointI;
                    Eigen::Vector2f lineItoJ = pointJ - pointI;

                    double coefficient = lineItoJ.dot(lineItoK) / lineItoK.squaredNorm();
                    Eigen::Vector2f dispJFromLine = lineItoJ - coefficient * lineItoK;

                    // Discard the line if any point J is further than one unit
                    // off the line.
                    if (dispJFromLine.norm() >= 1.0)
                    {
                        isStraightPath = false;
                        break;
                    }
                }

                if (isStraightPath)
                {
                    // Now that this line is known to be valid, compute the penalty
                    // for this path segment.

                    // Use an approximation of penalty from the Potrace paper.
                    const double x = pointK.x() - pointI.x();
                    const double y = pointK.y() - pointI.y();

                    const double xBar = (pointK + pointI).x() / 2.0;
                    const double yBar = (pointK + pointI).y() / 2.0;

                    // Compute expected values of all the terms below.
                    double expectedX = 0.0;
                    double expectedY = 0.0;
                    double expectedXY = 0.0;
                    double expectedXSquare = 0.0;
                    double expectedYSquare = 0.0;

                    for (int j = i; j <= k; j++)
                    {
                        const int xAtj = chain.Get(j).x;
                        const int yAtj = chain.Get(j).y;

                        expectedX += xAtj;
                        expectedY += yAtj;
                        expectedXY += xAtj * yAtj;
                        expectedXSquare += xAtj * xAtj;
                        expectedYSquare += yAtj * yAtj;
                    }

                    expectedX /= (k - i + 1);
                    expectedY /= (k - i + 1);
                    expectedXY /= (k - i + 1);
                    expectedXSquare /= (k - i + 1);
                    expectedYSquare /= (k - i + 1);

                    // Evaluate the penalty approximation from the Potrace paper.
                    const double a = expectedXSquare - 2 * xBar * expectedX + pow(xBar, 2.0);
                    const double b = expectedXY - xBar * expectedX - yBar * expectedY + xBar * yBar;
                    const double c = expectedYSquare - 2 * yBar * expectedY + pow(yBar, 2.0);

                    const double interior = c * pow(x, 2.0) + 2 * b * x * y + a * pow(y, 2.0);
                    const double penalty = std::sqrt(interior);

                    penalties(i, k) = penalty;
                }
            }
        }

        // Search for the shortest and least-penalty path using the penalties
        // matrix. Invalid paths are now identified by dummy values left over
        // in the matrix.
        FindBestPath(polyline, chain, penalties);

        mPolylines << polyline;
    }

    emit Finished();
}

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
void DiffusionCurveRenderer::Potrace::FindBestPath(QVector<Point>& bestPath, PixelChain chain, Eigen::MatrixXd penalties)
{
    const int nPoints = chain.GetLength();

    // Store a vector which identifies best paths and their penalties from
    // pixel i to the endpoint.
    Eigen::VectorXd bestPenalties(nPoints);
    std::vector<std::vector<int>> bestPaths;

    // Make sure to set initial values for both penalties and paths, including
    // a sensible initial value for the endpoint itself.
    bestPenalties.setConstant(-1);
    bestPenalties(nPoints - 1) = 0;

    for (int i = 0; i < nPoints; i++)
    {
        std::vector<int> nullPath;
        bestPaths.push_back(nullPath);
    }

    bestPaths.at(nPoints - 1).push_back(nPoints - 1);

    // Work backwards, finding best paths from the end back to the beginning
    // using a dynamic programming approach.
    for (int i = nPoints - 2; i >= 0; i--)
    {
        for (int j = i + 1; j < nPoints; j++)
        {
            if (penalties(i, j) != -1)
            {
                const double penaltyCandidate = penalties(i, j) + bestPenalties(j);
                std::vector<int> pathCandidate = bestPaths.at(j);
                pathCandidate.push_back(i);

                bool firstPath = bestPaths.at(i).size() == 0;
                bool shortPath = pathCandidate.size() < bestPaths.at(i).size();
                bool equalPath = pathCandidate.size() == bestPaths.at(i).size();
                bool cheapPath = penaltyCandidate < bestPenalties(i);

                // Check if this is a new best path for any of the above reasons.
                if (firstPath || shortPath || (equalPath && cheapPath))
                {
                    bestPenalties(i) = penaltyCandidate;
                    bestPaths.at(i) = pathCandidate;
                }
            }
        }
    }

    std::vector<int> bestPathIndices = bestPaths.front();
    std::reverse(bestPathIndices.begin(), bestPathIndices.end());

    // Convert the path indices into a polyline.
    bestPath.clear();
    for (int i = 0; i < bestPathIndices.size(); i++)
    {
        Point point = chain.Get(bestPathIndices.at(i));
        bestPath.push_back(point);
    }
}

const QVector<QVector<DiffusionCurveRenderer::Point>>& DiffusionCurveRenderer::Potrace::GetPolylines() const
{
    return mPolylines;
}

void DiffusionCurveRenderer::Potrace::Reset()
{
    mPolylines.clear();
}