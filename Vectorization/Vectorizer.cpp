#include "Vectorizer.h"
#include "imgui.h"

#include <Dependencies/Eigen/SparseCore>

#include <QDebug>

typedef Eigen::Vector2f Vector2;

Vectorizer::Vectorizer()
    : mSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mVectorizationStatus(VectorizationStatus::Ready)
    , mCannyUpperThreshold(200.0f)
    , mCannyLowerThreshold(80.0f)
    , mGaussianStack(nullptr)
    , mEdgeStack(nullptr)
    , mSelectedGaussianLayer(0)
    , mSelectedEdgeLayer(0)
    , mInit(false)
    , mUpdateInitialData(false)
{
    mBitmapRenderer = BitmapRenderer::instance();
}

void Vectorizer::load(const QString &path)
{
    mUpdateInitialData = false;
    clear();

    mOriginalImage = cv::imread(path.toStdString(), cv::IMREAD_REDUCED_COLOR_2);

    cv::Canny(mOriginalImage, mEdgeImage, mCannyUpperThreshold, mCannyLowerThreshold);

    mVectorizationStatus = VectorizationStatus::CreatingGaussianStack;
    mGaussianStack = new GaussianStack(mOriginalImage);

    mVectorizationStatus = VectorizationStatus::CreatingEdgeStack;
    mEdgeStack = new EdgeStack(mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);

    mVectorizationStatus = VectorizationStatus::TracingEdges;
    traceEdgePixels(mChains, mEdgeStack->layer(1), 5);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mChains.size();

    mSubWorkMode = SubWorkMode::ViewOriginalImage;
    mVectorizationStatus = VectorizationStatus::Finished;
    mInit = true;
    mUpdateInitialData = true;
}

void Vectorizer::drawGui()
{
    ImGui::Spacing();

    if (mVectorizationStatus == VectorizationStatus::CreatingGaussianStack)
    {
        ImGui::Text("Status: Creating Gaussian Stack...");
        return;

    } else if (mVectorizationStatus == VectorizationStatus::CreatingEdgeStack)
    {
        ImGui::Text("Status: Creating Edge Stack...");
        return;

    } else if (mVectorizationStatus == VectorizationStatus::TracingEdges)
    {
        ImGui::Text("Status: TracingEdges...");
        return;
    }

    if (!mInit)
        return;

    if (mUpdateInitialData)
    {
        mBitmapRenderer->setData(mOriginalImage, mOriginalImage.cols, mOriginalImage.rows, GL_BGR);
        mUpdateInitialData = false;
    }

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "SubWork Modes");

    int mode = (int) mSubWorkMode;

    if (ImGui::RadioButton("View Original Image", &mode, 0))
        mBitmapRenderer->setData(mOriginalImage, mOriginalImage.cols, mOriginalImage.rows, GL_BGR);

    if (ImGui::RadioButton("View Edges", &mode, 1))
        mBitmapRenderer->setData(mEdgeImage, mEdgeImage.cols, mEdgeImage.rows, GL_RED);

    if (ImGui::RadioButton("View Gaussian Stack", &mode, 2))
    {
        auto layer = mGaussianStack->layer(mSelectedGaussianLayer);
        mBitmapRenderer->setData(layer, layer.cols, layer.rows, GL_BGR);
    }

    if (ImGui::RadioButton("View Edge Stack", &mode, 3))
    {
        auto layer = mEdgeStack->layer(mSelectedEdgeLayer);
        mBitmapRenderer->setData(layer, layer.cols, layer.rows, GL_RED);
    }

    if (mSubWorkMode == SubWorkMode::ViewGaussianStack)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gaussian Stack Layers");

        if (ImGui::SliderInt("Layer##Gaussian", &mSelectedGaussianLayer, 0, mGaussianStack->height() - 1))
        {
            auto layer = mGaussianStack->layer(mSelectedGaussianLayer);
            mBitmapRenderer->setData(layer, layer.cols, layer.rows, GL_BGR);
        }
    }

    if (mSubWorkMode == SubWorkMode::ViewEdgeStack)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Edge Stack Layers");

        if (ImGui::SliderInt("Layer##Edge", &mSelectedEdgeLayer, 0, mEdgeStack->height() - 1))
        {
            auto layer = mEdgeStack->layer(mSelectedEdgeLayer);
            mBitmapRenderer->setData(layer, layer.cols, layer.rows, GL_RED);
        }
    }

    mSubWorkMode = SubWorkMode(mode);
}

Vectorizer *Vectorizer::instance()
{
    static Vectorizer instance;
    return &instance;
}

void Vectorizer::clear()
{
    if (mGaussianStack)
        delete mGaussianStack;

    if (mEdgeStack)
        delete mEdgeStack;

    mChains.clear();
}

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
void Vectorizer::traceEdgePixels(QVector<PixelChain> &chains, cv::Mat edges, int lengthThreshold)
{
    const int width = edges.cols;
    const int height = edges.rows;
    const int nEdgePixels = cv::countNonZero(edges);

    // Matrix denoting which edges have already been included in a pixel chain.
    Eigen::SparseMatrix<int> visited(width, height);

    // Matrix giving fast access to edge pixels.
    cv::Mat nonZeros;
    cv::findNonZero(edges, nonZeros);

    std::vector<PixelChain> growingChains;

    for (int i = 0; i < nEdgePixels; i++)
    {
        int row = nonZeros.at<cv::Point>(i).y;
        int col = nonZeros.at<cv::Point>(i).x;

        if (visited.coeffRef(col, row) != 0)
        {
            // Skip pixels that have already been used in an edge, to avoid
            // double-counting.
            continue;
        }

        PixelChain points;
        bool neighborFound = false;

        // Search through neighbours of this pixel until the end of the chain
        // has been reached.
        do
        {
            points.append(Point(col, row));
            visited.coeffRef(col, row) = 1;

            // The neighbourhood consists of pixels within one space of the
            // current pixel.
            const int startCol = col == 0 ? col : col - 1;
            const int endCol = col == width - 1 ? col : col + 1;
            const int startRow = row == 0 ? row : row - 1;
            const int endRow = row == height - 1 ? row : row + 1;

            neighborFound = false;

            // Search for unused pixels in the neighbourhood (adjacent) to the
            // current pixel until a first one is found.
            for (int x = startCol; x <= endCol && !neighborFound; x++)
            {
                for (int y = startRow; y <= endRow && !neighborFound; y++)
                {
                    uchar pixelValue = edges.at<uchar>(y, x);

                    if (pixelValue != 0 && visited.coeffRef(x, y) == 0.0)
                    {
                        // Note down the new pixel and stop looking.
                        row = y;
                        col = x;
                        neighborFound = true;
                    }
                }
            }
        } while (neighborFound);

        bool newChain = true;

        // Look for any existing chains that connect to this one, and merge
        // them into one longer chain.
        for (int j = 0; j < growingChains.size(); j++)
        {
            Point chainHead = growingChains.at(j).head();
            Point chainTail = growingChains.at(j).tail();

            Point newcomerHead = points.head();
            Point newcomerTail = points.tail();

            if (chainTail.isNeighbour(newcomerHead))
            {
                // Insert the new chain at the end of the old chain.
                growingChains.at(j).insertBack(points);
                newChain = false;
            } else if (chainTail.isNeighbour(newcomerTail))
            {
                // Insert the new chain backwards at the end of the old chain.
                growingChains.at(j).insertBack(points.reversed());
                newChain = false;
            } else if (chainHead.isNeighbour(newcomerTail))
            {
                // Insert the new chain at the front of the old one.
                growingChains.at(j).insertFront(points);
                newChain = false;
            } else if (chainHead.isNeighbour(newcomerHead))
            {
                // Insert the new chain backwards at the front of the old one.
                growingChains.at(j).insertFront(points.reversed());
                newChain = false;
            }
        }

        if (newChain)
        {
            growingChains.push_back(points);
        }
    }

    // Only keep chains that are at least as long as the threshold.
    for (int i = 0; i < growingChains.size(); i++)
    {
        PixelChain candidate = growingChains.at(i);

        if (candidate.length() >= lengthThreshold)
        {
            chains.push_back(candidate);
        }
    }
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
void Vectorizer::potrace(QVector<Point> &polyline, PixelChain chain)
{
    const int nPoints = chain.length();

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

            Vector2 pointI = chain.get(i).toVector();
            Vector2 pointK = chain.get(k).toVector();

            // Check that all points between I and K are close enough to the
            // straight line connecting them.
            for (int j = i + 1; j < k; j++)
            {
                Vector2 pointJ = chain.get(j).toVector();

                Vector2 lineItoK = pointK - pointI;
                Vector2 lineItoJ = pointJ - pointI;

                double coefficient = lineItoJ.dot(lineItoK) / lineItoK.squaredNorm();
                Vector2 dispJFromLine = lineItoJ - coefficient * lineItoK;

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
                    const int xAtj = chain.get(j).x;
                    const int yAtj = chain.get(j).y;

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
    findBestPath(polyline, chain, penalties);
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
void Vectorizer::findBestPath(QVector<Point> &bestPath, PixelChain chain, Eigen::MatrixXd penalties)
{
    const int nPoints = chain.length();

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
        Point point = chain.get(bestPathIndices.at(i));
        bestPath.push_back(point);
    }
}