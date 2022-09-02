#ifndef BLURPOINT_H
#define BLURPOINT_H

#include "Common.h"
#include <QVector2D>

class Bezier;

class BlurPoint
{
public:
    BlurPoint();

    void setParent(Bezier *newParent);
    QVector2D getPosition2D(float gap = BLUR_POINT_VISUAL_GAP) const;

public:
    Bezier *mParent;
    float mStrength;
    float mPosition;
    bool mSelected;
};

#endif // BLURPOINT_H