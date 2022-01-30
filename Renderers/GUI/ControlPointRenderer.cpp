#include "ControlPointRenderer.h"

ControlPointRenderer::ControlPointRenderer()
    : mPointRenderer(nullptr)
    , mZoomRatio(1.0f)
{}

ControlPointRenderer::~ControlPointRenderer()
{
    if (mPointRenderer)
        delete mPointRenderer;

    mPointRenderer = nullptr;
}

bool ControlPointRenderer::init()
{
    mPointRenderer = new PointRenderer;

    mPointRendererParameters.innerColor = QVector4D(240 / 255.0f, 240 / 255.0f, 240 / 255.0f, 1);
    mPointRendererParameters.outerColor = QVector4D(120 / 255.0f, 120 / 255.0f, 120 / 255.0f, 0.75f);

    return mPointRenderer->init();
}

void ControlPointRenderer::render(Curve *curve, const QMatrix4x4 &projectionMatrix)
{
    if (curve == nullptr)
        return;

    QVector<const ControlPoint *> controlPoints = curve->getControlPoints();

    for (int j = 0; j < controlPoints.size(); ++j) {
        mPointRendererParameters.point = controlPoints[j]->position();
        mPointRendererParameters.innerRadius = mZoomRatio * (controlPoints[j]->selected() ? 6 : 6);
        mPointRendererParameters.outerRadius = mZoomRatio * (controlPoints[j]->selected() ? 12 : 10);
        mPointRenderer->render(mPointRendererParameters, projectionMatrix);
    }
}

void ControlPointRenderer::setZoomRatio(float newZoomRatio)
{
    mZoomRatio = newZoomRatio;
}
