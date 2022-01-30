#ifndef BOUNDINGBOXRENDERER_H
#define BOUNDINGBOXRENDERER_H

#include <Curves/Curve.h>
#include <Renderers/GUI/Base/LineRenderer.h>
#include <Renderers/GUI/Base/RectangleRenderer.h>

class BoundingBoxRenderer
{
public:
    BoundingBoxRenderer();
    ~BoundingBoxRenderer();

    bool init();
    void render(Curve *curve, const QMatrix4x4 &projectionMatrix);
    void setZoomRatio(float newZoomRatio);

private:
    RectangleRenderer *mRectangleRenderer;
    LineRenderer *mLineRenderer;

    LineRenderer::Parameters mLineRendererParameters;
    RectangleRenderer::Parameters mRectangleRendererParameters;

    float mZoomRatio;
};

#endif // BOUNDINGBOXRENDERER_H
