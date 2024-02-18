#include "EdgeStack.h"
#include "GaussianStack.h"

#include "opencv2/imgproc/imgproc.hpp"

DiffusionCurveRenderer::EdgeStack::EdgeStack()
    : mProgress(0.0f)
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
void DiffusionCurveRenderer::EdgeStack::Run(GaussianStack* stack, double lowThreshold, double highThreshold)
{
    int stackHeight = stack->GetHeight();

    // Meaningless, nonzero initial value for number of detected edge pixels.
    int nonzeros = 1;

    for (int layer = 0; layer < stackHeight && nonzeros > 0; layer++)
    {
        this->mProgress = float(layer) / float(stackHeight - 1);

        cv::Mat image = stack->GetLayer(layer);

        cv::Mat edges;
        cv::Canny(image, edges, lowThreshold, highThreshold);

        this->mLevels.push_back(edges);

        nonzeros = cv::countNonZero(edges);
    }

    if (nonzeros == 0)
    {
        // Remove the top level if it had no detectable edge pixels.
        this->mLevels.pop_back();
    }

    // Trim any blurred images from the Gaussian stack beyond the point
    // where edges stopped being detectable.
    int edgeHeight = this->GetHeight();
    if (edgeHeight < stackHeight)
    {
        stack->Restrict(edgeHeight);
    }
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
cv::Mat DiffusionCurveRenderer::EdgeStack::GetLayer(int layer)
{
    return this->mLevels.at(layer);
}

float DiffusionCurveRenderer::EdgeStack::GetProgress() const
{
    return this->mProgress;
}