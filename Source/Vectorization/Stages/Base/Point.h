#pragma once

#include <Eigen/Core>
#include <functional>
#include <iostream>

namespace DiffusionCurveRenderer
{
    class Point
    {
      public:
        Point();
        Point(double X, double Y);
        double Dist(Point Other) const;
        double Sqdist(Point Other) const;
        Eigen::Vector2f ToVector() const;
        double X;
        double Y;

        double GetNorm() const;
        void Normalize();
        bool IsNeighbour(const Point Other);

        bool operator==(const Point& Rhs) const;
        bool operator!=(const Point& Rhs) const;
    };
}

namespace std
{
    template<>
    struct hash<DiffusionCurveRenderer::Point>
    {
        size_t operator()(const DiffusionCurveRenderer::Point& K) const
        {
            // Compute individual hash values for two data members and combine them using XOR and bit shifting
            return ((hash<double>()(K.X)) ^ (hash<double>()(K.Y)) >> 1);
        }
    };

    std::ostream& operator<<(std::ostream& strm, const DiffusionCurveRenderer::Point& P);
} // namespace std
