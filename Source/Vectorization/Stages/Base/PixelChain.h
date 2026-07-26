#pragma once

#include "Point.h"

#include <deque>

namespace DiffusionCurveRenderer
{
    class PixelChain
    {
      private:
        /*
         * Sequence of pixels, in numerical order, which form a continuous chain.
         */
        std::deque<Point> mPoints;

      public:
        /*
         * Create a new empty chain of pixels.
         */
        PixelChain();

        /*
         * Create a new chain of pixels which is a copy of <other>.
         *
         * param Other: Reference to another continuous pixel chain.
         */
        PixelChain(const PixelChain& Other);

        /*
         * Returns the number of pixels in the chain.
         */
        int GetLength();

        /*
         * Returns the first pixel position in the chain.
         */
        Point GetHead();

        /*
         * Returns the last pixel position in the chain.
         */
        Point GetTail();

        /*
         * Returns the <index>'th pixel position in the chain.
         */
        Point Get(int index);

        /*
         * Expands the pixel chain by attaching a new pixel to the front.
         *
         * param Point: A pixel location which is adjacent to the current head
         *              of the chain.
         */
        void Prepend(Point Point);

        /*
         * Expands the pixel chain by attaching a new pixel to the end.
         *
         * param Point: A pixel location which is adjacent to the current tail
         *              of the chain.
         */
        void Append(Point Point);

        /*
         * Reverses the order of pixels in this chain, in-place.
         */
        void Reverse();

        /*
         * Returns a copy of this pixel chain in reverse order. Does not modify
         * this pixel chain.
         */
        PixelChain Reversed();

        /*
         * Extends this pixel chain by adding an entire other chain to the front.
         * That is, if <other> is a1-a2-...-an and this is b1-b2-...-bm, then
         * this pixel chain will be mutated to become a1-a2-...-an-b1-b2...bm.
         *
         * The other chain will not be modified.
         *
         * param Other: Another chain of pixels to add to this one.
         */
        void InsertFront(PixelChain Other);

        /*
         * Extends this pixel chain by adding an entire other chain to the back.
         * That is, if <other> is a1-a2-...-an and this is b1-b2-...-bm, then
         * this pixel chain will be mutated to become b1-b2...bm-a1-a2-...-an.
         *
         * The other chain will not be modified.
         *
         * param Other: Another chain of pixels to add to this one.
         */
        void InsertBack(PixelChain Other);
    };
}
