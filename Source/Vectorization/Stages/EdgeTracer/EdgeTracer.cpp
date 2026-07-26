#include "EdgeTracer.h"

#include <Eigen/SparseCore>

DiffusionCurveRenderer::EdgeTracer::EdgeTracer(QObject* parent)
    : VectorizationStageBase(parent)
{}

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
void DiffusionCurveRenderer::EdgeTracer::Run(cv::Mat Edges, int LengthThreshold)
{
    const int Width = Edges.cols;
    const int Height = Edges.rows;
    const int nEdgePixels = cv::countNonZero(Edges);

    // Matrix denoting which edges have already been included in a pixel chain.
    Eigen::SparseMatrix<int> Visited(Width, Height);

    // Matrix giving fast access to edge pixels.
    cv::Mat NonZeros;
    cv::findNonZero(Edges, NonZeros);

    std::vector<PixelChain> GrowingChains;

    for (int i = 0; i < nEdgePixels; i++)
    {
        float Progress = float(i) / (nEdgePixels - 1);

        emit ProgressChanged(Progress);

        int Row = NonZeros.at<cv::Point>(i).y;
        int Col = NonZeros.at<cv::Point>(i).x;

        if (Visited.coeffRef(Col, Row) != 0)
        {
            // Skip pixels that have already been used in an edge, to avoid
            // double-counting.
            continue;
        }

        PixelChain Points;
        bool NeighborFound = false;

        // Search through neighbours of this pixel until the end of the chain
        // has been reached.
        do
        {
            Points.Append(Point(Col, Row));
            Visited.coeffRef(Col, Row) = 1;

            // The neighbourhood consists of pixels within one space of the
            // current pixel.
            const int StartCol = Col == 0 ? Col : Col - 1;
            const int EndCol = Col == Width - 1 ? Col : Col + 1;
            const int StartRow = Row == 0 ? Row : Row - 1;
            const int EndRow = Row == Height - 1 ? Row : Row + 1;

            NeighborFound = false;

            // Search for unused pixels in the neighbourhood (adjacent) to the
            // current pixel until a first one is found.
            for (int x = StartCol; x <= EndCol && !NeighborFound; x++)
            {
                for (int y = StartRow; y <= EndRow && !NeighborFound; y++)
                {
                    uchar pixelValue = Edges.at<uchar>(y, x);

                    if (pixelValue != 0 && Visited.coeffRef(x, y) == 0.0)
                    {
                        // Note down the new pixel and stop looking.
                        Row = y;
                        Col = x;
                        NeighborFound = true;
                    }
                }
            }
        } while (NeighborFound);

        bool NewChain = true;

        // Look for any existing chains that connect to this one, and merge
        // them into one longer chain.
        for (int j = 0; j < GrowingChains.size(); j++)
        {
            Point ChainHead = GrowingChains.at(j).GetHead();
            Point ChainTail = GrowingChains.at(j).GetTail();

            Point NewcomerHead = Points.GetHead();
            Point NewcomerTail = Points.GetTail();

            if (ChainTail.IsNeighbour(NewcomerHead))
            {
                // Insert the new chain at the end of the old chain.
                GrowingChains.at(j).InsertBack(Points);
                NewChain = false;
            }
            else if (ChainTail.IsNeighbour(NewcomerTail))
            {
                // Insert the new chain backwards at the end of the old chain.
                GrowingChains.at(j).InsertBack(Points.Reversed());
                NewChain = false;
            }
            else if (ChainHead.IsNeighbour(NewcomerTail))
            {
                // Insert the new chain at the front of the old one.
                GrowingChains.at(j).InsertFront(Points);
                NewChain = false;
            }
            else if (ChainHead.IsNeighbour(NewcomerHead))
            {
                // Insert the new chain backwards at the front of the old one.
                GrowingChains.at(j).InsertFront(Points.Reversed());
                NewChain = false;
            }
        }

        if (NewChain)
        {
            GrowingChains.push_back(Points);
        }
    }

    // Only keep chains that are at least as long as the threshold.
    for (int i = 0; i < GrowingChains.size(); i++)
    {
        PixelChain Candidate = GrowingChains.at(i);

        if (Candidate.GetLength() >= LengthThreshold)
        {
            mChains.push_back(Candidate);
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
