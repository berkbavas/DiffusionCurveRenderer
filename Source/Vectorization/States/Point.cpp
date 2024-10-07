#include "Point.h"

#include <cmath>
#include <iostream>

DiffusionCurveRenderer::Point::Point()
    : Point(0, 0)
{
}

DiffusionCurveRenderer::Point::Point(double x, double y)
{
    this->x = x;
    this->y = y;
}

double DiffusionCurveRenderer::Point::Dist(const Point o) const
{
    return sqrt((x - o.x) * (x - o.x) + (y - o.y) * (y - o.y));
}

void DiffusionCurveRenderer::Point::Normalize()
{
    double norm = this->GetNorm();
    if (norm != 0)
    {
        x /= norm;
        y /= norm;
    }
}

double DiffusionCurveRenderer::Point::GetNorm() const
{
    return sqrt(x * x + y * y);
}

bool DiffusionCurveRenderer::Point::IsNeighbour(const Point other)
{
    // True if the two points are diagonally connected; false if they are
    // separated by more than 1 pixels on any axis.
    return this->Sqdist(other) < 3.0;
}

double DiffusionCurveRenderer::Point::Sqdist(Point o) const
{
    return (x - o.x) * (x - o.x) + (y - o.y) * (y - o.y);
}

Eigen::Vector2f DiffusionCurveRenderer::Point::ToVector() const
{
    Eigen::Vector2f converted;
    converted << this->x, this->y;

    return converted;
}

bool DiffusionCurveRenderer::Point::operator==(const Point& rhs) const
{
    return x == rhs.x && y == rhs.y;
}

bool DiffusionCurveRenderer::Point::operator!=(const Point& rhs) const
{
    return !(rhs == *this);
}

std::ostream& std::operator<<(std::ostream& strm, const DiffusionCurveRenderer::Point& p)
{
    return strm << "(" << p.x << ", " << p.y << ")";
}