#ifndef CONTROLPOINTRENDERER_H
#define CONTROLPOINTRENDERER_H

#include <Curves/Curve.h>
#include <Renderers/GUI/Base/PointRenderer.h>

class ControlPointRenderer
{
public:
    ControlPointRenderer();
    ~ControlPointRenderer();

    bool init();
    void render(Curve *curve, const QMatrix4x4 &projectionMatrix);
    void setZoomRatio(float newZoomRatio);

private:
    PointRenderer *mPointRenderer;
    PointRenderer::Parameters mPointRendererParameters;

    float mZoomRatio;
};

#endif // CONTROLPOINTRENDERER_H
