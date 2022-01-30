#ifndef LINERENDERER_H
#define LINERENDERER_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <Renderers/Base/Ticks.h>

class LineRenderer : protected QOpenGLFunctions
{
public:
    LineRenderer();
    ~LineRenderer();

    enum LineStyle { Solid = 0, Dashed = 1 };

    struct Parameters
    {
        QVector2D startingPoint;
        QVector2D endPoint;
        QVector4D color;
        LineStyle lineStyle;
        float thickness;
        float dashLength;
        float gapLength;
    };

    bool init();
    void render(const Parameters &parameters, const QMatrix4x4 &projectionMatrix);

private:
    QOpenGLShaderProgram *mShader;

    Ticks *mDenseTicks;
    Ticks *mRareTicks;

    int mProjectionMatrixLocation;
    int mStartingPointLocation;
    int mEndPointLocation;
    int mColorLocation;
    int mThicknessLocation;
    int mLineStyleLocation;
    int mLineLengthLocation;
    int mDashLengthLocation;
    int mGapLengthLocation;
    int mRareTicksDeltaLocation;
    int mDenseTicksDeltaLocation;
};

#endif // LINERENDERER_H
