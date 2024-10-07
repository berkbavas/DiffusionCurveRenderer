#include "EdgeTracer.h"

#include <Eigen/SparseCore>

DiffusionCurveRenderer::EdgeTracer::EdgeTracer(QObject* parent)
    : VectorizationStageBase(parent)
{
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
void DiffusionCurveRenderer::EdgeTracer::Run(cv::Mat edges, int lengthThreshold)
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
        float progress = float(i) / (nEdgePixels - 1);

        emit ProgressChanged(progress);

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
            points.Append(Point(col, row));
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
            Point chainHead = growingChains.at(j).GetHead();
            Point chainTail = growingChains.at(j).GetTail();

            Point newcomerHead = points.GetHead();
            Point newcomerTail = points.GetTail();

            if (chainTail.IsNeighbour(newcomerHead))
            {
                // Insert the new chain at the end of the old chain.
                growingChains.at(j).InsertBack(points);
                newChain = false;
            }
            else if (chainTail.IsNeighbour(newcomerTail))
            {
                // Insert the new chain backwards at the end of the old chain.
                growingChains.at(j).InsertBack(points.Reversed());
                newChain = false;
            }
            else if (chainHead.IsNeighbour(newcomerTail))
            {
                // Insert the new chain at the front of the old one.
                growingChains.at(j).InsertFront(points);
                newChain = false;
            }
            else if (chainHead.IsNeighbour(newcomerHead))
            {
                // Insert the new chain backwards at the front of the old one.
                growingChains.at(j).InsertFront(points.Reversed());
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

        if (candidate.GetLength() >= lengthThreshold)
        {
            mChains.push_back(candidate);
        }
    }

    emit Finished();
}

const QList<DiffusionCurveRenderer::PixelChain>& DiffusionCurveRenderer::EdgeTracer::GetChains() const
{
    return mChains;
}

void DiffusionCurveRenderer::EdgeTracer::Reset()
{
    mChains.clear();
}
