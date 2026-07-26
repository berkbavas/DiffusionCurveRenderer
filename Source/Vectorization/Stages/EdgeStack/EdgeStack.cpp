#include "EdgeStack.h"

#include "opencv2/imgproc/imgproc.hpp"

DiffusionCurveRenderer::EdgeStack::EdgeStack(QObject* pParent)
    : VectorizationStageBase(pParent)
{}

/**
 * Creates a stack of edge images from a Gaussian scale space.
 *
 * Runs Canny edge detection on each image in the Gaussian stack.
 * Edge detection uses low and high thresholds as specified by
 * parameters.
 *
 * param stack: A stack of images from a Gaussian scale space.
 * param lowThreshold: Low edge strength threshold for Canny edges.
 * param highThreshold: High edge strength threshold for Canny edges.
 */
void DiffusionCurveRenderer::EdgeStack::Run(GaussianStack* pStack, double LowThreshold, double HighThreshold)
{
    const auto StackHeight = pStack->GetHeight();

    // Meaningless, nonzero initial value for number of detected edge pixels.
    int Nonzeros = 1;

    for (int Layer = 0; Layer < StackHeight && Nonzeros > 0; Layer++)
    {
        float Progress = float(Layer) / float(StackHeight - 1);
        emit ProgressChanged(Progress);

        cv::Mat Image = pStack->GetLayer(Layer);

        cv::Mat Edges;
        cv::Canny(Image, Edges, LowThreshold, HighThreshold);

        this->mLevels.push_back(Edges);

        Nonzeros = cv::countNonZero(Edges);
    }

    if (Nonzeros == 0)
    {
        // Remove the top level if it had no detectable edge pixels.
        this->mLevels.pop_back();
    }

    // @berkbavas: Do we really need this?
    // Trim any blurred images from the Gaussian stack beyond the point
    // where edges stopped being detectable.
    // int edgeHeight = this->GetHeight();

    // if (edgeHeight < stackHeight)
    // {
    //     stack->Restrict(edgeHeight);
    // }

    emit Finished();
}

/*
 * Returns the number of levels in the edge stack.
 */
int DiffusionCurveRenderer::EdgeStack::GetHeight()
{
    return this->mLevels.size();
}

/*
 * Returns the image of edges at the <layer>'th layer.
 */
cv::Mat DiffusionCurveRenderer::EdgeStack::GetLayer(int Layer)
{
    return this->mLevels.at(Layer);
}

void DiffusionCurveRenderer::EdgeStack::Reset()
{
    this->mLevels.clear();
}