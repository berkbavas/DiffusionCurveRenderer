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
        Point(double x, double y);
        double Dist(Point o) const;
        double Sqdist(Point o) const;
        Eigen::Vector2f ToVector() const;
        double x;
        double y;

        double GetNorm() const;
        void Normalize();
        bool IsNeighbour(const Point other);

        bool operator==(const Point& rhs) const;
        bool operator!=(const Point& rhs) const;
    };
}

namespace std
{
    template<>
    struct hash<DiffusionCurveRenderer::Point>
    {
        size_t operator()(const DiffusionCurveRenderer::Point& k) const
        {
            // Compute individual hash values for two data members and combine them using XOR and bit shifting
            return ((hash<double>()(k.x)) ^ (hash<double>()(k.y)) >> 1);
        }
    };

    std::ostream& operator<<(std::ostream& strm, const DiffusionCurveRenderer::Point& p);
} // namespace std
