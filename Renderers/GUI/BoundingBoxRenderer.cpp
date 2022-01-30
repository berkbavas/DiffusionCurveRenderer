#include "BoundingBoxRenderer.h"

BoundingBoxRenderer::BoundingBoxRenderer()
    : mRectangleRenderer(nullptr)
    , mLineRenderer(nullptr)
    , mZoomRatio(1.0)
{}

BoundingBoxRenderer::~BoundingBoxRenderer()
{
    if (mRectangleRenderer)
        delete mRectangleRenderer;

    if (mLineRenderer)
        delete mLineRenderer;

    mRectangleRenderer = nullptr;
    mLineRenderer = nullptr;
}

bool BoundingBoxRenderer::init()
{
    mRectangleRenderer = new RectangleRenderer;
    mLineRenderer = new LineRenderer;

    if (!mRectangleRenderer->init())
        return false;

    if (!mLineRenderer->init())
        return false;

    mLineRendererParameters.color = QVector4D(0.6f, 0.6f, 0.6f, 1.0f);
    mLineRendererParameters.dashLength = 4;
    mLineRendererParameters.gapLength = 4;
    mLineRendererParameters.lineStyle = LineRenderer::Dashed;

    mRectangleRendererParameters.fillColor = QVector4D(0.6f, 0.6f, 0.6f, 1);
    mRectangleRendererParameters.borderEnabled = false;
    mRectangleRendererParameters.borderWidth = 0;
    mRectangleRendererParameters.borderColor = QVector4D(1, 1, 1, 1);

    return true;
}

void BoundingBoxRenderer::render(Curve *curve, const QMatrix4x4 &projectionMatrix)
{
    if (curve == nullptr)
        return;

    QRectF boundingBox = curve->getBoundingBox();

    // Dashed Lines
    {
        mLineRendererParameters.thickness = mZoomRatio * 1;

        mLineRendererParameters.startingPoint = QVector2D(boundingBox.topLeft()) + mZoomRatio * QVector2D(0, 5);
        mLineRendererParameters.endPoint = QVector2D(boundingBox.topRight()) + mZoomRatio * QVector2D(0, 5);
        mLineRenderer->render(mLineRendererParameters, projectionMatrix);

        mLineRendererParameters.startingPoint = QVector2D(boundingBox.topRight()) + mZoomRatio * QVector2D(5, 0);
        mLineRendererParameters.endPoint = QVector2D(boundingBox.bottomRight()) + mZoomRatio * QVector2D(5, 0);
        mLineRenderer->render(mLineRendererParameters, projectionMatrix);

        mLineRendererParameters.startingPoint = QVector2D(boundingBox.bottomRight()) + mZoomRatio * QVector2D(0, 5);
        mLineRendererParameters.endPoint = QVector2D(boundingBox.bottomLeft()) + mZoomRatio * QVector2D(0, 5);
        mLineRenderer->render(mLineRendererParameters, projectionMatrix);

        mLineRendererParameters.startingPoint = QVector2D(boundingBox.bottomLeft()) + mZoomRatio * QVector2D(5, 0);
        mLineRendererParameters.endPoint = QVector2D(boundingBox.topLeft()) + mZoomRatio * QVector2D(5, 0);
        mLineRenderer->render(mLineRendererParameters, projectionMatrix);
    }

    // Corners
    {
        QSizeF size = mZoomRatio * QSizeF(10, 10);

        mRectangleRendererParameters.rectangle = QRectF(boundingBox.topLeft(), size);
        mRectangleRenderer->render(mRectangleRendererParameters, projectionMatrix);

        mRectangleRendererParameters.rectangle = QRectF(boundingBox.topRight(), size);
        mRectangleRenderer->render(mRectangleRendererParameters, projectionMatrix);

        mRectangleRendererParameters.rectangle = QRectF(boundingBox.bottomLeft(), size);
        mRectangleRenderer->render(mRectangleRendererParameters, projectionMatrix);

        mRectangleRendererParameters.rectangle = QRectF(boundingBox.bottomRight(), size);
        mRectangleRenderer->render(mRectangleRendererParameters, projectionMatrix);
    }
}

void BoundingBoxRenderer::setZoomRatio(float newZoomRatio)
{
    mZoomRatio = newZoomRatio;
}
