#include "Point.h"

#include <cmath>
#include <iostream>

DiffusionCurveRenderer::Point::Point()
    : Point(0, 0)
{}

DiffusionCurveRenderer::Point::Point(double X, double Y)
{
    this->X = X;
    this->Y = Y;
}

double DiffusionCurveRenderer::Point::Dist(const Point Other) const
{
    return sqrt((X - Other.X) * (X - Other.X) + (Y - Other.Y) * (Y - Other.Y));
}

void DiffusionCurveRenderer::Point::Normalize()
{
    const auto Norm = this->GetNorm();
    if (Norm != 0)
    {
        X /= Norm;
        Y /= Norm;
    }
}

double DiffusionCurveRenderer::Point::GetNorm() const
{
    return sqrt(X * X + Y * Y);
}

bool DiffusionCurveRenderer::Point::IsNeighbour(const Point Other)
{
    // True if the two points are diagonally connected; false if they are
    // separated by more than 1 pixels on any axis.
    return this->Sqdist(Other) < 3.0;
}

double DiffusionCurveRenderer::Point::Sqdist(Point Other) const
{
    return (X - Other.X) * (X - Other.X) + (Y - Other.Y) * (Y - Other.Y);
}

Eigen::Vector2f DiffusionCurveRenderer::Point::ToVector() const
{
    Eigen::Vector2f converted;
    converted << this->X, this->Y;

    return converted;
}

bool DiffusionCurveRenderer::Point::operator==(const Point& Rhs) const
{
    return X == Rhs.X && Y == Rhs.Y;
}

bool DiffusionCurveRenderer::Point::operator!=(const Point& Rhs) const
{
    return !(Rhs == *this);
}

std::ostream& std::operator<<(std::ostream& strm, const DiffusionCurveRenderer::Point& p)
{
    return strm << "(" << p.X << ", " << p.Y << ")";
}
