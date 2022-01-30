#include "LineRenderer.h"

#include <Helper.h>

LineRenderer::LineRenderer()
    : mShader(nullptr)
    , mDenseTicks(nullptr)
    , mRareTicks(nullptr)
{}

LineRenderer::~LineRenderer()
{
    if (mShader)
        mShader->deleteLater();

    if (mDenseTicks)
        delete mDenseTicks;

    if (mRareTicks)
        delete mRareTicks;

    mShader = nullptr;
    mDenseTicks = nullptr;
    mRareTicks = nullptr;
}

bool LineRenderer::init()
{
    initializeOpenGLFunctions();

    mShader = new QOpenGLShaderProgram;

    if (!mShader->addShaderFromSourceCode(QOpenGLShader::Vertex, Helper::getBytes(":/Resources/Shaders/Line/VertexShader.vert"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Fragment, Helper::getBytes(":/Resources/Shaders/Line/FragmentShader.frag"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Geometry, Helper::getBytes(":/Resources/Shaders/Line/GeometryShader.geom"))
        || !mShader->link() || !mShader->bind()) {
        qCritical() << mShader->log();
        return false;
    }

    // Uniforms Variables
    mProjectionMatrixLocation = mShader->uniformLocation("projectionMatrix");
    mStartingPointLocation = mShader->uniformLocation("startingPoint");
    mEndPointLocation = mShader->uniformLocation("endPoint");
    mColorLocation = mShader->uniformLocation("color");
    mThicknessLocation = mShader->uniformLocation("thickness");
    mRareTicksDeltaLocation = mShader->uniformLocation("rareTicksDelta");
    mDenseTicksDeltaLocation = mShader->uniformLocation("denseTicksDelta");
    mLineStyleLocation = mShader->uniformLocation("lineStyle");
    mLineLengthLocation = mShader->uniformLocation("lineLength");
    mDashLengthLocation = mShader->uniformLocation("dashLength");
    mGapLengthLocation = mShader->uniformLocation("gapLength");

    // Vertex Attribute Location
    mShader->bindAttributeLocation("vs_Tick", 0);

    // Ticks
    mRareTicks = new Ticks(0, 1, 10);
    mRareTicks->create();

    mDenseTicks = new Ticks(0, 1, 1000);
    mDenseTicks->create();

    return true;
}

void LineRenderer::render(const Parameters &parameters, const QMatrix4x4 &projectionMatrix)
{
    mShader->bind();

    mShader->setUniformValue(mProjectionMatrixLocation, projectionMatrix);
    mShader->setUniformValue(mRareTicksDeltaLocation, mRareTicks->ticksDelta());
    mShader->setUniformValue(mDenseTicksDeltaLocation, mDenseTicks->ticksDelta());

    mShader->setUniformValue(mStartingPointLocation, parameters.startingPoint);
    mShader->setUniformValue(mEndPointLocation, parameters.endPoint);
    mShader->setUniformValue(mColorLocation, parameters.color);
    mShader->setUniformValue(mThicknessLocation, parameters.thickness);
    mShader->setUniformValue(mLineStyleLocation, parameters.lineStyle);
    mShader->setUniformValue(mLineLengthLocation, parameters.startingPoint.distanceToPoint(parameters.endPoint));
    mShader->setUniformValue(mDashLengthLocation, parameters.dashLength);
    mShader->setUniformValue(mGapLengthLocation, parameters.gapLength);

    if (parameters.lineStyle == LineStyle::Solid) {
        mRareTicks->bind();
        glDrawArrays(GL_POINTS, 0, mRareTicks->size());
        mRareTicks->release();

    } else if (parameters.lineStyle == LineStyle::Dashed) {
        mDenseTicks->bind();
        glDrawArrays(GL_POINTS, 0, mDenseTicks->size());
        mDenseTicks->release();
    }

    mShader->release();
}
