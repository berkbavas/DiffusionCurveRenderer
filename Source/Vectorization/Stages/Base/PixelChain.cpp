#include "PixelChain.h"

/*
 * Create a new empty chain of pixels.
 */
DiffusionCurveRenderer::PixelChain::PixelChain()
{
    this->mPoints.clear();
}

/*
 * Create a new chain of pixels which is a copy of <other>.
 *
 * param other: Reference to another continuous pixel chain.
 */
DiffusionCurveRenderer::PixelChain::PixelChain(const PixelChain& Other)
{
    // Use the implicit copy constructor for deques.
    this->mPoints = Other.mPoints;
}

/*
 * Returns the number of pixels in the chain.
 */
int DiffusionCurveRenderer::PixelChain::GetLength()
{
    return this->mPoints.size();
}

/*
 * Returns the first pixel position in the chain.
 */
DiffusionCurveRenderer::Point DiffusionCurveRenderer::PixelChain::GetHead()
{
    return this->mPoints.front();
}

/*
 * Returns the last pixel position in the chain.
 */
DiffusionCurveRenderer::Point DiffusionCurveRenderer::PixelChain::GetTail()
{
    return this->mPoints.back();
}

/*
 * Returns the <index>'th pixel position in the chain.
 */
DiffusionCurveRenderer::Point DiffusionCurveRenderer::PixelChain::Get(int Index)
{
    return this->mPoints.at(Index);
}

/*
 * Expands the pixel chain by attaching a new pixel to the front.
 *
 * param point: A pixel location which is adjacent to the current head
 *              of the chain.
 */
void DiffusionCurveRenderer::PixelChain::Prepend(Point Point)
{
    this->mPoints.push_front(Point);
}

/*
 * Expands the pixel chain by attaching a new pixel to the end.
 *
 * param point: A pixel location which is adjacent to the current tail
 *              of the chain.
 */
void DiffusionCurveRenderer::PixelChain::Append(Point Point)
{
    this->mPoints.push_back(Point);
}

/*
 * Reverses the order of pixels in this chain, in-place.
 */
void DiffusionCurveRenderer::PixelChain::Reverse()
{
    std::reverse(this->mPoints.begin(), this->mPoints.end());
}

/*
 * Returns a copy of this pixel chain in reverse order. Does not modify
 * this pixel chain.
 */
DiffusionCurveRenderer::PixelChain DiffusionCurveRenderer::PixelChain::Reversed()
{
    PixelChain pCopy = *this;
    pCopy.Reverse();
    return pCopy;
}

/*
 * Extends this pixel chain by adding an entire other chain to the front.
 * That is, if <other> is a1-a2-...-an and this is b1-b2-...-bm, then
 * this pixel chain will be mutated to become a1-a2-...-an-b1-b2...bm.
 *
 * The other chain will not be modified.
 *
 * param other: Another chain of pixels to add to this one.
 */
void DiffusionCurveRenderer::PixelChain::InsertFront(PixelChain Other)
{
    std::deque<Point> Copy = Other.mPoints;

    for (int I = 0; I < Other.GetLength(); I++)
    {
        this->Prepend(Copy.back());
        Copy.pop_back();
    }
}

/*
 * Extends this pixel chain by adding an entire other chain to the back.
 * That is, if <other> is a1-a2-...-an and this is b1-b2-...-bm, then
 * this pixel chain will be mutated to become b1-b2...bm-a1-a2-...-an.
 *
 * The other chain will not be modified.
 *
 * param other: Another chain of pixels to add to this one.
 */
void DiffusionCurveRenderer::PixelChain::InsertBack(PixelChain Other)
{
    std::deque<Point> Copy = Other.mPoints;

    for (int I = 0; I < Other.GetLength(); I++)
    {
        this->Append(Copy.front());
        Copy.pop_front();
    }
}
