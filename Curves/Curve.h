#ifndef CURVE_H
#define CURVE_H

#include "ColorPoint.h"
#include "ControlPoint.h"
#include "Types.h"

#include <QVector4D>
#include <QVector>

class Curve : public QObject
{
public:
    Curve();
    virtual ~Curve();

    virtual QVector<const ControlPoint *> getControlPoints() const;
    virtual const ControlPoint *getControlPoint(int index) const;
    virtual QVector<QVector2D> getControlPointPositions() const;
    virtual bool addControlPoint(ControlPoint *controlPoint, bool append = true);

    virtual void removeControlPoint(int index);
    virtual void removeControlPoint(ControlPoint *controlPoint);

    virtual int getSize() const;
    virtual int getOrder() const;
    virtual int getDegree() const;

    virtual QVector<const ColorPoint *> getLeftColorPoints() const;
    virtual QVector<const ColorPoint *> getRightColorPoints() const;

    virtual bool addColorPoint(ColorPoint *colorPoint);

    virtual const ColorPoint *getLeftColorPoint(int index) const;
    virtual const ColorPoint *getRightColorPoint(int index) const;

    virtual void removeLeftColorPoint(int index);
    virtual void removeRightColorPoint(int index);
    virtual void removeColorPoint(ColorPoint *controlPoint);

    virtual QVector4D leftColorAt(float t) const;
    virtual QVector4D rightColorAt(float t) const;

    virtual void orderLeftColorPoints();
    virtual void orderRightColorPoints();

    virtual QVector<QVector4D> getLeftColors() const;
    virtual QVector<QVector4D> getRightColors() const;

    virtual QVector<float> getLeftColorPositions() const;
    virtual QVector<float> getRightColorPositions() const;

    virtual const ColorPoint *getClosestColorPoint(const QVector2D &nearbyPoint) const;
    virtual const ControlPoint *getClosestControlPoint(const QVector2D &nearbyPoint) const;

    virtual void deselectAllControlPoints();
    virtual float distanceToPoint(const QVector2D &point, int intervals = 1000) const;

    virtual void translate(QVector2D translation);
    virtual void translate(QPointF translation);

    virtual float length(int intervals = 100) const;

    virtual QRectF getBoundingBox(int intervals = 100) const;

    virtual QVector2D valueAt(float t) const = 0;
    virtual QVector2D tangentAt(float t) const = 0;
    virtual QVector2D normalAt(float t) const = 0;
    virtual float parameterAt(const QVector2D &point, int intervals = 1000) const;
    virtual void scale(float factor);

    const bool &selected() const;
    void setSelected(bool newSelected);

    const int &z() const;
    void setZ(int newZ);

    const QVector4D &contourColor() const;
    void setContourColor(const QVector4D &newContourColor);

    bool contourColorEnabled() const;
    void setContourColorEnabled(bool newContourColorEnabled);

    float contourThickness() const;
    void setContourThickness(float newContourThickness);

    float diffusionWidth() const;
    void setDiffusionWidth(float newDiffusionWidth);

protected:
    QVector<ControlPoint *> mControlPoints;

    QVector<ColorPoint *> mLeftColorPoints;
    QVector<ColorPoint *> mRightColorPoints;

    bool mContourColorEnabled;
    QVector4D mContourColor;

    float mContourThickness;
    float mDiffusionWidth;
    bool mSelected;
    int mZ;

private:
    void updateControlPointIndices();
    static QVector2D findMeanCenter(const QVector<QVector2D> &points);
    static QVector<QVector2D> translate(const QVector<QVector2D> &points, const QVector2D &translation);
    static void findLineOfBestFit(const QVector<QVector2D> &points, QVector2D &startingPoint, QVector2D &direction, int segments = 1000);
    static float averageDistanceToLine(const QVector<QVector2D> &points, const QVector2D &startingPoint, const QVector2D &direction);
    static float perpendicularAt(const QVector2D &startingPoint, const QVector2D &direction, const QVector2D &subject);
    static float distanceToLineSegment(const QVector2D &startingPoint, const QVector2D &endPoint, const QVector2D &subject, int intervals = 100);
};

#endif // CURVE_H
