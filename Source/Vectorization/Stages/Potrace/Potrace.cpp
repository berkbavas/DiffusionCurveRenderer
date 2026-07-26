#include "Potrace.h"

DiffusionCurveRenderer::Potrace::Potrace(QObject* pParent)
    : VectorizationStageBase(pParent)
{}

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
void DiffusionCurveRenderer::Potrace::Run(const QVector<PixelChain>& Chains)
{
    for (int I = 0; I < Chains.size(); I++)
    {
        const auto Progress = float(I) / (Chains.size() - 1);

        emit ProgressChanged(Progress);

        PixelChain Chain = Chains[I];
        QVector<Point> Polyline;

        const int nPoints = Chain.GetLength();

        // Each entry i, j represents the penalty of a straight line
        // from pixel i directly to pixel j.
        Eigen::MatrixXd Penalties(nPoints, nPoints);
        // Set a recognizable dummy value to signify no penalty is set.
        Penalties.setConstant(-1.0);

        // First, find a penalty between each pair of pixels, or leave the dummy
        // value if two pixels cannot be connected by a straight line.

        // Iterate over all triplets i, j, k, such that 0 <= i < j < k <= nPoints.
        for (int I = 0; I < nPoints; I++)
        {
            for (int K = I + 1; K < nPoints; K++)
            {
                bool IsStraightPath = true;

                Eigen::Vector2f PointI = Chain.Get(I).ToVector();
                Eigen::Vector2f PointK = Chain.Get(K).ToVector();

                // Check that all points between I and K are close enough to the
                // straight line connecting them.
                for (int J = I + 1; J < K; J++)
                {
                    Eigen::Vector2f PointJ = Chain.Get(J).ToVector();

                    Eigen::Vector2f LineToK = PointK - PointI;
                    Eigen::Vector2f LineToJ = PointJ - PointI;

                    double Coefficient = LineToJ.dot(LineToK) / LineToK.squaredNorm();
                    Eigen::Vector2f DispJFromLine = LineToJ - Coefficient * LineToK;

                    // Discard the line if any point J is further than one unit
                    // off the line.
                    if (DispJFromLine.norm() >= 1.0)
                    {
                        IsStraightPath = false;
                        break;
                    }
                }

                if (IsStraightPath)
                {
                    // Now that this line is known to be valid, compute the penalty
                    // for this path segment.

                    // Use an approximation of penalty from the Potrace paper.
                    const double X = PointK.x() - PointI.x();
                    const double Y = PointK.y() - PointI.y();

                    const double XBar = (PointK + PointI).x() / 2.0;
                    const double YBar = (PointK + PointI).y() / 2.0;

                    // Compute expected values of all the terms below.
                    double ExpectedX = 0.0;
                    double ExpectedY = 0.0;
                    double ExpectedXY = 0.0;
                    double ExpectedXSquare = 0.0;
                    double ExpectedYSquare = 0.0;

                    for (int J = I; J <= K; J++)
                    {
                        const int xAtJ = Chain.Get(J).X;
                        const int yAtJ = Chain.Get(J).Y;

                        ExpectedX += xAtJ;
                        ExpectedY += yAtJ;
                        ExpectedXY += xAtJ * yAtJ;
                        ExpectedXSquare += xAtJ * xAtJ;
                        ExpectedYSquare += yAtJ * yAtJ;
                    }

                    ExpectedX /= (K - I + 1);
                    ExpectedY /= (K - I + 1);
                    ExpectedXY /= (K - I + 1);
                    ExpectedXSquare /= (K - I + 1);
                    ExpectedYSquare /= (K - I + 1);

                    // Evaluate the penalty approximation from the Potrace paper.
                    const double A = ExpectedXSquare - 2 * XBar * ExpectedX + pow(XBar, 2.0);
                    const double B = ExpectedXY - XBar * ExpectedX - YBar * ExpectedY + XBar * YBar;
                    const double C = ExpectedYSquare - 2 * YBar * ExpectedY + pow(YBar, 2.0);

                    const double Interior = C * pow(X, 2.0) + 2 * B * X * Y + A * pow(Y, 2.0);
                    const double Penalty = std::sqrt(Interior);

                    Penalties(I, K) = Penalty;
                }
            }
        }

        // Search for the shortest and least-penalty path using the penalties
        // matrix. Invalid paths are now identified by dummy values left over
        // in the matrix.
        FindBestPath(Polyline, Chain, Penalties);

        mPolylines << Polyline;
    }

    emit Finished();
}

/* This code taken from https://github.com/zhuethanca/DiffusionCurves
 *
 * Finds a sequence of pixels, forming a polyline, which approximates
 * the pixel chain <chain> with the minimum number of segments and with
 * the minimum penalty given the matrix <penalties>.
 *
 * param Polyline: Return value, a sequence of points approximating the chain.
 * param Chain: Continuous chain of pixels to be approximated by a polyline.
 * param Penalties: Matrix in which the entry i, j represents the penalty of
 *                  a straight line between pixels i and j, or -1 if they are
 *                  not connectable by a straight line.
 */
void DiffusionCurveRenderer::Potrace::FindBestPath(QVector<Point>& Polyline, PixelChain Chain, Eigen::MatrixXd Penalties)
{
    const int nPoints = Chain.GetLength();

    // Store a vector which identifies best paths and their penalties from
    // pixel i to the endpoint.
    Eigen::VectorXd BestPenalties(nPoints);
    std::vector<std::vector<int>> BestPaths;

    // Make sure to set initial values for both penalties and paths, including
    // a sensible initial value for the endpoint itself.
    BestPenalties.setConstant(-1);
    BestPenalties(nPoints - 1) = 0;

    for (int I = 0; I < nPoints; I++)
    {
        std::vector<int> NullPath;
        BestPaths.push_back(NullPath);
    }

    BestPaths.at(nPoints - 1).push_back(nPoints - 1);

    // Work backwards, finding best paths from the end back to the beginning
    // using a dynamic programming approach.
    for (int I = nPoints - 2; I >= 0; I--)
    {
        for (int j = I + 1; j < nPoints; j++)
        {
            if (Penalties(I, j) != -1)
            {
                const double PenaltyCandidate = Penalties(I, j) + BestPenalties(j);
                std::vector<int> PathCandidate = BestPaths.at(j);
                PathCandidate.push_back(I);

                bool FirstPath = BestPaths.at(I).size() == 0;
                bool ShortPath = PathCandidate.size() < BestPaths.at(I).size();
                bool EqualPath = PathCandidate.size() == BestPaths.at(I).size();
                bool CheapPath = PenaltyCandidate < BestPenalties(I);

                // Check if this is a new best path for any of the above reasons.
                if (FirstPath || ShortPath || (EqualPath && CheapPath))
                {
                    BestPenalties(I) = PenaltyCandidate;
                    BestPaths.at(I) = PathCandidate;
                }
            }
        }
    }

    std::vector<int> BestPathIndices = BestPaths.front();
    std::reverse(BestPathIndices.begin(), BestPathIndices.end());

    // Convert the path indices into a polyline.
    Polyline.clear();
    for (int i = 0; i < BestPathIndices.size(); i++)
    {
        Point point = Chain.Get(BestPathIndices.at(i));
        Polyline.push_back(point);
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
