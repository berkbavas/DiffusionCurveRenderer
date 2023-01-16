#include "Bezier.h"
#include "Common.h"

DiffusionCurveRenderer::Bezier::Bezier()
    : mContourColor(DEFAULT_CONTOUR_COLOR)
    , mContourThickness(DEFAULT_CONTOUR_THICKNESS)
    , mDiffusionWidth(DEFAULT_DIFFUSION_WIDTH)
    , mDiffusionGap(DEFAULT_DIFFUSION_GAP)
    , mSelected(false)
    , mDepth(0)
    , mVoid(false)
{}

DiffusionCurveRenderer::Bezier::~Bezier()
{
    for (int i = 0; i < mControlPoints.size(); ++i)
        if (mControlPoints[i])
            delete mControlPoints[i];

    for (int i = 0; i < mLeftColorPoints.size(); ++i)
        if (mLeftColorPoints[i])
            delete mLeftColorPoints[i];

    for (int i = 0; i < mRightColorPoints.size(); ++i)
        if (mRightColorPoints[i])
            delete mRightColorPoints[i];

    for (int i = 0; i < mBlurPoints.size(); ++i)
        if (mBlurPoints[i])
            delete mBlurPoints[i];
}

QVector<QVector2D> DiffusionCurveRenderer::Bezier::GetControlPointPositions() const
{
    QVector<QVector2D> positions;

    for (const auto& point : mControlPoints)
        positions << point->mPosition;

    return positions;
}

void DiffusionCurveRenderer::Bezier::AddControlPoint(ControlPoint* controlPoint)
{
    mControlPoints << controlPoint;
}

void DiffusionCurveRenderer::Bezier::RemoveControlPoint(int index)
{
    if (0 <= index && index < mControlPoints.size())
    {
        delete mControlPoints[index];
        mControlPoints.removeAt(index);
    }
}

void DiffusionCurveRenderer::Bezier::RemoveControlPoint(ControlPoint* controlPoint)
{
    for (int i = 0; i < mControlPoints.size(); ++i)
    {
        ControlPoint* point = mControlPoints[i];

        if (point == controlPoint)
        {
            RemoveControlPoint(i);
            return;
        }
    }
}

void DiffusionCurveRenderer::Bezier::AddColorPoint(ColorPoint* colorPoint)
{
    switch (colorPoint->mDirection)
    {
    case ColorPoint::Direction::Left: {
        if (colorPoint)
        {
            mLeftColorPoints << colorPoint;
            colorPoint->SetParent(this);
            OrderLeftColorPoints();
        }

        break;
    }
    case ColorPoint::Direction::Right: {
        if (colorPoint)
        {
            mRightColorPoints << colorPoint;
            colorPoint->SetParent(this);
            OrderRightColorPoints();
        }
        break;
    }
    }
}

void DiffusionCurveRenderer::Bezier::AddBlurPoint(BlurPoint* blurPoint)
{
    mBlurPoints << blurPoint;
    blurPoint->SetParent(this);
}

QVector<DiffusionCurveRenderer::ColorPoint*> DiffusionCurveRenderer::Bezier::GetAllColorPoints() const
{
    QVector<ColorPoint*> all;
    all << mLeftColorPoints;
    all << mRightColorPoints;

    return all;
}

void DiffusionCurveRenderer::Bezier::RemoveLeftColorPoint(int index)
{
    delete mLeftColorPoints[index];
    mLeftColorPoints.removeAt(index);
    OrderLeftColorPoints();
}

void DiffusionCurveRenderer::Bezier::RemoveRightColorPoint(int index)
{
    delete mRightColorPoints[index];
    mRightColorPoints.removeAt(index);
    OrderRightColorPoints();
}

void DiffusionCurveRenderer::Bezier::RemoveColorPoint(ColorPoint* point)
{
    switch (point->mDirection)
    {
    case ColorPoint::Direction::Left:
    {
        for (int i = 0; i < mLeftColorPoints.size(); ++i)
        {
            if (point == mLeftColorPoints[i])
            {
                RemoveLeftColorPoint(i);
                return;
            }
        }
        break;
    }
    case ColorPoint::Direction::Right:
    {
        for (int i = 0; i < mRightColorPoints.size(); ++i)
        {
            if (point == mRightColorPoints[i])
            {
                RemoveRightColorPoint(i);
                return;
            }
        }
        break;
    }
    }
}

void DiffusionCurveRenderer::Bezier::RemoveBlurPoint(BlurPoint* blurPoint)
{
    for (int i = 0; i < mBlurPoints.size(); ++i)
    {
        if (blurPoint == mBlurPoints[i])
        {
            RemoveBlurPoint(i);
            return;
        }
    }
}

void DiffusionCurveRenderer::Bezier::RemoveBlurPoint(int index)
{
    delete mBlurPoints[index];
    mBlurPoints.removeAt(index);
    OrderBlurPoints();
}

QVector4D DiffusionCurveRenderer::Bezier::GetLeftColorAt(float t) const
{
    if (mLeftColorPoints.size() == 0 || mLeftColorPoints.size() == 1)
        return QVector4D(0, 0, 0, 0);

    for (int i = 1; i < mLeftColorPoints.size(); ++i)
    {
        if (mLeftColorPoints[i - 1]->mPosition <= t && t <= mLeftColorPoints[i]->mPosition)
        {
            float s = (t - mLeftColorPoints[i - 1]->mPosition) / (mLeftColorPoints[i]->mPosition - mLeftColorPoints[i - 1]->mPosition);
            return mLeftColorPoints[i - 1]->mColor + s * (mLeftColorPoints[i]->mColor - mLeftColorPoints[i - 1]->mColor);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

QVector4D DiffusionCurveRenderer::Bezier::GetRightColorAt(float t) const
{
    if (mRightColorPoints.size() == 0 || mRightColorPoints.size() == 1)
        return QVector4D(0, 0, 0, 0);

    for (int i = 1; i < mRightColorPoints.size(); ++i)
    {
        if (mRightColorPoints[i - 1]->mPosition <= t && t <= mRightColorPoints[i]->mPosition)
        {
            float s = (t - mRightColorPoints[i - 1]->mPosition) / (mRightColorPoints[i]->mPosition - mRightColorPoints[i - 1]->mPosition);
            return mRightColorPoints[i - 1]->mColor + s * (mRightColorPoints[i]->mColor - mRightColorPoints[i - 1]->mColor);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

void DiffusionCurveRenderer::Bezier::OrderLeftColorPoints()
{
    if (mLeftColorPoints.size() == 0 || mLeftColorPoints.size() == 1)
        return;

    QList<ColorPoint*> orderedColorPoints;

    orderedColorPoints << mLeftColorPoints[0];

    for (int i = 1; i < mLeftColorPoints.size(); ++i)
    {
        ColorPoint* currentColorPoint = mLeftColorPoints[i];

        if (orderedColorPoints.last()->mPosition <= currentColorPoint->mPosition)
            orderedColorPoints << currentColorPoint;
        else
            for (int j = 0; j < orderedColorPoints.size(); j++)
                if (currentColorPoint->mPosition < orderedColorPoints[j]->mPosition)
                {
                    orderedColorPoints.insert(j, currentColorPoint);
                    break;
                }
    }

    mLeftColorPoints = orderedColorPoints;
}

void DiffusionCurveRenderer::Bezier::OrderRightColorPoints()
{
    if (mRightColorPoints.size() == 0 || mRightColorPoints.size() == 1)
        return;

    QList<ColorPoint*> orderedColorPoints;

    orderedColorPoints << mRightColorPoints[0];

    for (int i = 1; i < mRightColorPoints.size(); ++i)
    {
        ColorPoint* currentColorPoint = mRightColorPoints[i];

        if (orderedColorPoints.last()->mPosition <= currentColorPoint->mPosition)
            orderedColorPoints << currentColorPoint;
        else
            for (int j = 0; j < orderedColorPoints.size(); j++)
                if (currentColorPoint->mPosition < orderedColorPoints[j]->mPosition)
                {
                    orderedColorPoints.insert(j, currentColorPoint);
                    break;
                }
    }

    mRightColorPoints = orderedColorPoints;
}

void DiffusionCurveRenderer::Bezier::OrderBlurPoints()
{
    if (mBlurPoints.size() == 0 || mBlurPoints.size() == 1)
        return;

    QList<BlurPoint*> orderedPoints;

    orderedPoints << mBlurPoints[0];

    for (int i = 1; i < mBlurPoints.size(); ++i)
    {
        BlurPoint* currentPoint = mBlurPoints[i];

        if (orderedPoints.last()->mPosition <= currentPoint->mPosition)
            orderedPoints << currentPoint;
        else
            for (int j = 0; j < orderedPoints.size(); j++)
                if (currentPoint->mPosition < orderedPoints[j]->mPosition)
                {
                    orderedPoints.insert(j, currentPoint);
                    break;
                }
    }

    mBlurPoints = orderedPoints;
}

QVector<QVector4D> DiffusionCurveRenderer::Bezier::GetLeftColors() const
{
    QVector<QVector4D> leftColors;
    for (int i = 0; i < mLeftColorPoints.size(); i++)
        leftColors << mLeftColorPoints[i]->mColor;

    return leftColors;
}

QVector<QVector4D> DiffusionCurveRenderer::Bezier::GetRightColors() const
{
    QVector<QVector4D> rightColors;
    for (int i = 0; i < mRightColorPoints.size(); i++)
        rightColors << mRightColorPoints[i]->mColor;

    return rightColors;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetBlurPointStrengths() const
{
    QVector<float> blurStrengths;
    for (int i = 0; i < mBlurPoints.size(); i++)
        blurStrengths << mBlurPoints[i]->mStrength;

    return blurStrengths;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetLeftColorPositions() const
{
    QVector<float> colorPointPositions;

    for (int i = 0; i < mLeftColorPoints.size(); i++)
        colorPointPositions << mLeftColorPoints[i]->mPosition;

    return colorPointPositions;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetRightColorPositions() const
{
    QVector<float> colorPointPositions;

    for (int i = 0; i < mRightColorPoints.size(); i++)
        colorPointPositions << mRightColorPoints[i]->mPosition;

    return colorPointPositions;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetBlurPointPositions()
{
    QVector<float> positions;

    for (int i = 0; i < mBlurPoints.size(); i++)
        positions << mBlurPoints[i]->mPosition;

    return positions;
}

DiffusionCurveRenderer::ColorPoint* DiffusionCurveRenderer::Bezier::GetClosestColorPoint(const QVector2D& point) const
{
    QVector<ColorPoint*> allColorPoints;
    allColorPoints << mLeftColorPoints << mRightColorPoints;

    if (allColorPoints.size() == 0)
        return nullptr;

    float minimumDistance = std::numeric_limits<float>::infinity();
    int index = 0;

    for (int i = 0; i < allColorPoints.size(); ++i)
    {
        float distance = allColorPoints[i]->GetPosition2D().distanceToPoint(point);

        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            index = i;
        }
    }

    return allColorPoints[index];
}

DiffusionCurveRenderer::BlurPoint* DiffusionCurveRenderer::Bezier::GetClosestBlurPoint(const QVector2D& nearbyPoint) const
{
    if (mBlurPoints.size() == 0)
        return nullptr;

    float minimumDistance = std::numeric_limits<float>::infinity();
    int index = 0;

    for (int i = 0; i < mBlurPoints.size(); ++i)
    {
        float distance = mBlurPoints[i]->GetPosition2D().distanceToPoint(nearbyPoint);

        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            index = i;
        }
    }

    return mBlurPoints[index];
}

DiffusionCurveRenderer::ControlPoint* DiffusionCurveRenderer::Bezier::GetClosestControlPoint(const QVector2D& nearbyPoint) const
{
    if (mControlPoints.size() == 0)
        return nullptr;

    float minimumDistance = std::numeric_limits<float>::infinity();
    int index = 0;

    for (int i = 0; i < mControlPoints.size(); ++i)
    {
        float distance = mControlPoints[i]->mPosition.distanceToPoint(nearbyPoint);

        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            index = i;
        }
    }

    return mControlPoints[index];
}

QVector2D DiffusionCurveRenderer::Bezier::ValueAt(float t) const
{
    QVector2D value = QVector2D(0, 0);
    int n = mControlPoints.size() - 1;

    for (int i = 0; i <= n; i++)
        value += Choose(n, i) * pow(t, i) * pow(1 - t, n - i) * mControlPoints[i]->mPosition;

    return value;
}

QVector2D DiffusionCurveRenderer::Bezier::TangentAt(float t) const
{
    QVector2D tangent = QVector2D(0, 0);
    int order = mControlPoints.size() - 1;

    for (int i = 0; i <= order - 1; i++)
    {
        float coefficient = Choose(order - 1, i) * pow(t, i) * pow(1 - t, order - 1 - i);
        tangent += coefficient * (mControlPoints.at(i)->mPosition - mControlPoints.at(i + 1)->mPosition);
    }

    tangent.normalize();

    return tangent;
}

QVector2D DiffusionCurveRenderer::Bezier::NormalAt(float t) const
{
    QVector2D tangent = TangentAt(t);

    return QVector2D(-tangent.y(), tangent.x());
}

void DiffusionCurveRenderer::Bezier::Scale(float scaleFactor)
{
    for (auto& point : mControlPoints)
        point->mPosition = scaleFactor * point->mPosition;
}

DiffusionCurveRenderer::Bezier* DiffusionCurveRenderer::Bezier::DeepCopy()
{
    Bezier* newCurve = new Bezier;
    newCurve->mContourColor = mContourColor;
    newCurve->mContourThickness = mContourThickness;
    newCurve->mDiffusionWidth = mDiffusionWidth;
    newCurve->mSelected = mSelected;
    newCurve->mDepth = mDepth;

    for (const auto& point : mControlPoints)
    {
        ControlPoint* newPoint = new ControlPoint;
        newPoint->mPosition = point->mPosition;
        newPoint->mSelected = point->mSelected;

        newCurve->AddControlPoint(newPoint);
    }

    for (const auto& point : mLeftColorPoints)
    {
        ColorPoint* newPoint = new ColorPoint;
        newPoint->mPosition = point->mPosition;
        newPoint->mSelected = point->mSelected;
        newPoint->mColor = point->mColor;
        newPoint->mDirection = point->mDirection;
        newPoint->SetParent(newCurve);

        newCurve->AddColorPoint(newPoint);
    }

    for (const auto& point : mRightColorPoints)
    {
        ColorPoint* newPoint = new ColorPoint;
        newPoint->mPosition = point->mPosition;
        newPoint->mSelected = point->mSelected;
        newPoint->mColor = point->mColor;
        newPoint->mDirection = point->mDirection;
        newPoint->SetParent(newCurve);

        newCurve->AddColorPoint(newPoint);
    }

    for (const auto& point : mBlurPoints)
    {
        BlurPoint* newPoint = new BlurPoint;
        newPoint->mPosition = point->mPosition;
        newPoint->mSelected = point->mSelected;
        newPoint->mStrength = point->mStrength;
        newPoint->SetParent(newCurve);

        newCurve->AddBlurPoint(newPoint);
    }

    return newCurve;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetCoefficients() const
{
    QVector<float> coefficients;
    int n = mControlPoints.size() - 1;

    for (int i = 0; i <= n; ++i)
        coefficients << Choose(n, i);

    return coefficients;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetDerivativeCoefficients() const
{
    QVector<float> coefficients;
    int n = mControlPoints.size() - 1;

    for (int i = 0; i <= n - 1; ++i)
        coefficients << Choose(n - 1, i);

    return coefficients;
}

float DiffusionCurveRenderer::Bezier::Factorial(int n) const
{
    double result = 1.0;

    for (int i = 1; i <= n; ++i)
        result *= i;

    return result;
}

float DiffusionCurveRenderer::Bezier::Choose(int n, int k) const
{
    return Factorial(n) / (Factorial(k) * Factorial(n - k));
}

const QList<DiffusionCurveRenderer::ColorPoint*>& DiffusionCurveRenderer::Bezier::GetLeftColorPoints() const
{
    return mLeftColorPoints;
}

const QList<DiffusionCurveRenderer::ColorPoint*>& DiffusionCurveRenderer::Bezier::GetRightColorPoints() const
{
    return mRightColorPoints;
}

const QList<DiffusionCurveRenderer::BlurPoint*>& DiffusionCurveRenderer::Bezier::GetBlurPoints() const
{
    return mBlurPoints;
}

const QList<DiffusionCurveRenderer::ControlPoint*>& DiffusionCurveRenderer::Bezier::GetControlPoints() const
{
    return mControlPoints;
}

int DiffusionCurveRenderer::Bezier::GetOrder() const
{
    return mControlPoints.size();
}

int DiffusionCurveRenderer::Bezier::GetDegree() const
{
    return mControlPoints.size() - 1;
}

int DiffusionCurveRenderer::Bezier::GetSize() const
{
    return mControlPoints.size();
}