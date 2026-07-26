#include "GaussianStack.h"

DiffusionCurveRenderer::GaussianStack::GaussianStack(QObject* pParent)
    : VectorizationStageBase(pParent)
{}

/**
 * Construct a Gaussian scale space representing the image passed in.
 *
 * Repeatedly applies a Gaussian filter with radius sigma starting from 0
 * and increasing by <sigmaStep> (default 0.4) each level. The stack stops
 * when the image's standard deviation drops below <stdDevCutoff> (default
 * 40) or, if a max height is specified, at that height.
 *
 * param image: An OpenCV matrix containing an RGB image.
 * param stdDevCutoff: The minimum standard deviation of a blurred image
 *                     that will be used in the stack.
 * param maxHeight: The maximum height of the stack.
 * param sigmaStep: The increase in Gaussian filter widths between each level.
 */
void DiffusionCurveRenderer::GaussianStack::Run(cv::Mat Image, double StdDevCutoff, int MaxHeight, double SigmaStep)
{
    this->mLevels.clear();

    double Sigma = 0.4;

    int RemainingHeight = MaxHeight;

    do
    {
        float Progress = float(MaxHeight - RemainingHeight) / float(MaxHeight);
        emit ProgressChanged(Progress);

        // Define parameters for the next level of Gaussian filter.
        const int Radius = std::ceil(2 * Sigma);
        const int Width = 2 * Radius + 1;

        cv::Mat Blurred;
        cv::GaussianBlur(Image, Blurred, cv::Size(Width, Width), Sigma, Sigma);

        // Compute the image's standard deviation, averaged over three channels.
        //        cv::Scalar mean, stdDevChannels;
        //        cv::meanStdDev(blurred, mean, stdDevChannels);
        //        stdDev = (stdDevChannels[0] + stdDevChannels[1] + stdDevChannels[2]) / 3.0;

        Sigma += SigmaStep;

        this->mLevels.push_back(Blurred);

        RemainingHeight -= 1;
    } while (RemainingHeight != 0);

    emit Finished();
}

/*
 * Returns the number of levels in the scale space.
 */
int DiffusionCurveRenderer::GaussianStack::GetHeight()
{
    return this->mLevels.size();
}

/*
 * Restricts the height of the stack to <layers> layers, removing any
 * images above that level.
 */
void DiffusionCurveRenderer::GaussianStack::Restrict(int Layers)
{
    this->mLevels.resize(Layers);
}

/*
 * Returns the blurred RGB image at the <layer>'th layer.
 */
cv::Mat DiffusionCurveRenderer::GaussianStack::GetLayer(int Layer)
{
    return this->mLevels.at(Layer);
}

void DiffusionCurveRenderer::GaussianStack::Reset()
{
    mLevels.clear();
}
